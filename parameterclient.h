#ifndef JACKPARAMETERCLIENT_H
#define JACKPARAMETERCLIENT_H

#include "eventprocessorclient.h"
#include "parameterprocessor.h"
#include "jackthread.h"
#include <QGraphicsRectItem>

class ParameterThread;

/**
  Using this class:
  - provide an AudioProcessor and a MidiProcessor object (these could be identical if you use a class that implements both interfaces)
  - register any parameters you will need in your client, before activating the client
  - in the method AudioProcessor::processAudio() and in the MidiProcessor::process...() methods use the methods
    processGetIntParameter() and processGetDoubleParameter() to get the current value of a parameter
  - if you want to change a parameter from inside the process thread (i.e., if you want to change a parameter depending on
    an incoming MIDI controller value) call one of the processChangeIntParameter() or processChangeDoubleParameter() methods
  - connect to the ParameterClient::changedIntParameter() and ParameterClient::changedDoubleParameter() signals to show parameter changes in your GUI
  - use the slots ParameterClient::changeIntParameter() and ParameterClient::changeDoubleParameter() to change
    parameter values from the GUI thread
  - if you want to get the current value of a parameter from the GUI call getIntParameter() or getDoubleParameter()

    Internally this class keeps two sets of all registered parameters. One set is used for access from the process thread,
    the other one is being accessed from the GUI thread. Both sets are kept in sync via lock-free ringbuffers and the
    associated ParameterThread and its signals.

    Parameter changed from within the process thread are synchronized once per process cycle.
    Due to the nature of the communication between GUI and process thread there are always short phases
    in which both parameter sets are not equal. As parameters can be changed through events from inside
    the process thread (like MIDI events) as well as through event from the GUI thread (like user input)
    it can also happen that two parameter change messages in opposite directions are on their way at the
    same time, which would result in permanently putting both parameter sets out of sync, if they
    would not all be synchronized from process thread to GUI thread at the end of each process cycle.

    Note: make sure to always call the appropriately typed methods for a parameter, according to the parameter's registered type.
  */
class ParameterClient : public EventProcessorClient
{
    Q_OBJECT
public:
    struct ParameterChange {
        int id;
        double value;
        jack_nframes_t time;
    };

    ParameterClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, EventProcessor *eventProcessor, ParameterProcessor *parameterProcessor, size_t ringBufferSize = 1024);

    /**
      These methods save/load the current values of the parameter set.
      */
    virtual void saveState(QDataStream &stream);
    /**
      This method should only be called when the client is not active.
      */
    virtual void loadState(QDataStream &stream);

    int getNrOfParameters() const;
    const ParameterProcessor::Parameter & getParameter(int parameterId) const;
    /**
      Use this method to get the current parameter values from
      the GUI thread if you don't get their values from the corresponding signal below.
      */
    double getParameterValue(int parameterId);

    /**
      Reimplemented from JackClient.

      Returns a graphics item that allows changing the registered parameters
      from the GUI thread.
      */
    virtual QGraphicsItem * createGraphicsItem();
signals:
    /**
      This signal will be triggered iff the respective parameter's value
      is changed.

      Connect to this signal in the GUI thread to be notified
      when a parameter has been changed from either the process
      or the GUI thread.

      Note: the signal is only triggered once each process cycle
      with the same parameterId.
      */
    void changedParameterValue(int parameterId, double value);
    /**
      This signal will be triggered after all changed parameter
      signals (see above) have been triggered for the last
      process cycle.
      */
    void changedParameters();
public slots:
    /**
      Connect to or call this slot to communicate parameter changes
      from the GUI thread to the process thread.

      getParameterValue() will still return the old value until
      the change has been sent to the process thread and
      changedParameterValue() has been triggered.
      */
    void changeParameterValue(int parameterId, double value);
protected:
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
      Reimplemented from EventProcessorClient.

      Normally you do not need to reimplement this method.
      Reimplementing it might be necessary, if you want to handle
      an additional stream of data in parallel to the existing
      audio, midi and parameter data handled in processAudio(),
      processMidi() and this method, respectively.

      In that case, you have to call getPortBuffers() and getMidiBuffers()
      if you reimplement this method.
      */
    virtual bool process(jack_nframes_t nframes);

    virtual bool processParameters(jack_nframes_t start, jack_nframes_t end, jack_nframes_t nframes);

    void synchronizeChangedParametersWithGui();
private slots:
    /**
      This slot is connected to the corresponding signal of ParameterThread
      to keep the non-process thread set of parameters in sync with the process thread's.
      */
    void onChangedParameterValue(int parameterId, double value);
private:
    ParameterProcessor *parameterProcessor;
    QVector<ParameterProcessor::Parameter> parameters;
    JackRingBuffer<ParameterChange> ringBufferFromProcessToGui, ringBufferFromGuiToProcess;
    ParameterThread *thread;
};

class ParameterThread : public JackThread {
    Q_OBJECT
public:
    ParameterThread(ParameterClient *client, JackRingBuffer<ParameterClient::ParameterChange> *ringBufferFromProcessToGui, JackRingBuffer<ParameterClient::ParameterChange> *ringBufferFromGuiToProcess);
signals:
    /**
      You do not need to connect to this signal. Connect
      to ParameterClient's signal with the same name instead.
      It will be triggered correspondingly.
      */
    void changedParameterValue(int parameterId, double value);
    void changedParameters();
protected:
    void processDeferred();
private:
    JackRingBuffer<ParameterClient::ParameterChange> *ringBufferFromProcessToGui, *ringBufferFromGuiToProcess;
};

class GraphicsContinuousControlItem;

class ParameterGraphicsItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    ParameterGraphicsItem(ParameterClient *client, QGraphicsItem *parent = 0);
protected:
    virtual void focusInEvent(QFocusEvent * event);
    virtual void focusOutEvent(QFocusEvent * event);
private slots:
    void onGuiChangedParameterValue(double value);
    void onClientChangedParameterValue(int parameterId, double value);
private:
    ParameterClient *client;
    QMap<QObject*, int> mapSenderToId;
    QVector<GraphicsContinuousControlItem*> controls;
};

#endif // JACKPARAMETERCLIENT_H
