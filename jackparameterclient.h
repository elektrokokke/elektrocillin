#ifndef JACKPARAMETERCLIENT_H
#define JACKPARAMETERCLIENT_H

#include "midiprocessorclient.h"
#include "jackthread.h"

class JackParameterThread;

/**
  Using this class:
  - provide an AudioProcessor and a MidiProcessor object (these could be identical if you use a class that implements both interfaces)
  - register any parameters you will need in your client, before activating the client
  - in the method AudioProcessor::processAudio() and in the MidiProcessor::process...() methods use the methods
    processGetIntParameter() and processGetDoubleParameter() to get the current value of a parameter
  - if you want to change a parameter from inside the process thread (i.e., if you want to change a parameter depending on
    an incoming MIDI controller value) call one of the processChangeIntParameter() or processChangeDoubleParameter() methods
  - connect to the JackParameterClient::changedIntParameter() and JackParameterClient::changedDoubleParameter() signals to show parameter changes in your GUI
  - use the slots JackParameterClient::changeIntParameter() and JackParameterClient::changeDoubleParameter() to change
    parameter values from the GUI thread
  - if you want to get the current value of a parameter from the GUI call getIntParameter() or getDoubleParameter()

    Internally this class keeps two sets of all registered parameters. One set is used for access from the process thread,
    the other one is being accessed from the GUI thread. Both sets are kept in sync via lock-free ringbuffers and the
    associated JackParameterThread and its signals.

    Parameter changed from within the process thread are synchronized once per process cycle.
    Due to the nature of the communication between GUI and process thread there are always short phases
    in which both parameter sets are not equal. As parameters can be changed through events from inside
    the process thread (like MIDI events) as well as through event from the GUI thread (like user input)
    it can also happen that two parameter change messages in opposite directions are on their way at the
    same time, which would result in permanently putting both parameter sets out of sync, if they
    would not all be synchronized from process thread to GUI thread at the end of each process cycle.

    Note: make sure to always call the appropriately typed methods for a parameter, according to the parameter's registered type.
  */
class JackParameterClient : public MidiProcessorClient
{
    Q_OBJECT
public:
    enum ParameterType {
        TYPE_INT,
        TYPE_DOUBLE
    };
    struct Parameter {
        int id;
        ParameterType type;
        union {
            int intValue;
            double doubleValue;
        };
        jack_nframes_t time;
    };
    JackParameterClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, unsigned int channelMask = (1 << 16) - 1, size_t ringBufferSize = 1024);

    /**
      These methods save/load the current values of the parameter set.
      */
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);

    /**
      Use these methods to get the current parameter values from
      the GUI thread if you do get their values from the corresponding signals below.
      */
    int getIntParameter(int parameterId);
    double getDoubleParameter(int parameterId);
signals:
    /**
      These signals will be triggered iff the respective parameter's value
      is changed.
      */
    void changedIntParameter(int parameterId, int value);
    void changedDoubleParameter(int parameterId, double value);
public slots:
    /**
      Call these slots to communicate parameter changed to
      the process thread.
      */
    void changeIntParameter(int parameterId, int value);
    void changeDoubleParameter(int parameterId, double value);
protected:
    /**
      Register a parameter with the given type.

      @return a unique numeric identifier of the parameter, which can be
        used in the other methods accessing parameters
      */
    int registerParameter(ParameterType type);
    /**
      Call these methods from one of the other process...() methods in derived classes to
      change a parameter value.

      The given parameter will be synchronized with the non-process/non-RT thread (i.e., the GUI thread).

      If you want to read the current parameter value from the process thread, use the
      processGetIntParameter() and processGetDoubleParameter() methods.

      Which method you should use is determined by the type with which the parameter has
      been registered (using the registerParameter() method).

      @param parameterId the numeric identifier of the parameter whose value should be changed,
        as returned by the registerParameter() method
      @param value the new value of the parameter in question
      @param time the time (relative to the current audio buffer) at which the parameter value should change
      */
    void processChangeIntParameter(int parameterId, int value);
    void processChangeDoubleParameter(int parameterId, double value);
    /**
      Use these methods from one of the other process...() methods in derived classes to
      access the current value of a given parameter.

      The returned values will be up-to-date values synchronized with the GUI thread, if called
      from the processAudio() method of the client's AudioProcessor and from any of the
      process...() methods of the client's MidiProcessor.

      When a parameter is changed from the GUI thread, this will be reflected there.
      Changes from the process thread via the processChange...() methods will also be
      reflected by these methods.
      */
    int processGetIntParameter(int parameterId);
    double processGetDoubleParameter(double parameterId);
    /**
      Reimplemented from MidiProcessorClient.

      This starts the associated thread.
      You must call this in your reimplementation.
      */
    virtual bool init();
    /**
      Reimplemented from JackClient.

      This stops the associated thread.
      You must call this in your reimplementation.
      */
    virtual void deinit();
    /**
      Reimplemented from MidiProcessorClient.

      Normally you do not need to reimplement this method.
      Reimplementing it might be necessary, if you want to handle
      an additional stream of data in parallel to the existing
      audio, midi and parameter data handled in processAudio(),
      processMidi() and this method, respectively.

      In that case, you have to call getPortBuffers() and getMidiBuffers()
      if you reimplement this method.
      */
    virtual bool process(jack_nframes_t nframes);
private slots:
    /**
      These slots are connected to the corresponding signals of JackParameterThread
      to keep the non-process thread set of parameters in sync with the process thread's.
      */
    void onChangedIntParameter(int parameterId, int value);
    void onChangedDoubleParameter(int parameterId, double value);
private:
    QVector<Parameter> parameters, parametersProcess;
    JackRingBuffer<Parameter> ringBufferFromProcessToGui, ringBufferFromGuiToProcess;
    JackParameterThread *thread;

    void synchronizeChangedParametersWithGui();
};

class JackParameterThread : public JackThread {
    Q_OBJECT
public:
    JackParameterThread(JackParameterClient *client, JackRingBuffer<JackParameterClient::Parameter> *ringBufferFromProcessToGui, JackRingBuffer<JackParameterClient::Parameter> *ringBufferFromGuiToProcess);
signals:
    /**
      Do not connect to these signals. They will be triggered
      at the end of every process cycle, regardless of wether
      the parameter's value has been changed or not.

      This is to avoid the parameter sets from both threads to
      get out of sync.

      Use the JackParameterClient signals instead to be notified
      only when the values really changed.
      */
    void changedIntParameter(int parameterId, int value);
    void changedDoubleParameter(int parameterId, double value);
protected:
    void processDeferred();
private:
    JackRingBuffer<JackParameterClient::Parameter> *ringBufferFromProcessToGui, *ringBufferFromGuiToProcess;
};

#endif // JACKPARAMETERCLIENT_H
