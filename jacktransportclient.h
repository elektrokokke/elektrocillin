#ifndef JACKTRANSPORTCLIENT_H
#define JACKTRANSPORTCLIENT_H

#include "jackthreadeventprocessorclient.h"
#include "graphicslabelitem.h"

class JackTransportClient : public JackThreadEventProcessorClient
{
public:
    class TimebaseEvent : public RingBufferEvent {
    public:
        double beatsPerMinute;
        int beatsPerBar;
        int beatType;
    };

    JackTransportClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~JackTransportClient();

    virtual JackClientFactory * getFactory();

    QGraphicsItem * createGraphicsItem();
protected:
    /**
      Reimplemented from JackThreadEventProcessorClient.

      Registers the JackTimebaseCallback to become timebase master.
      */
    virtual bool init();
    /**
      Reimplemented from EventProcessorClient.

      Queries for current transport position and sends it to the associated
      JackTransportThread. Then normal processing is performed (the superclass
      implementation of process() is called).
      */
    virtual bool process(jack_nframes_t nframes);
    /**
      Reimplemented from MidiProcessorClient.

      The processNoteOn() method changes the tempo based on the note number.
      This is rather for testing purposes then for real use.
      The other methods do nothing and are only reimplemented because
      this class does not provide a MidiProcessor (and would therefore
      crash if any MIDI messages arrived through the input).

      Of course, another solution would be to just remove the MIDI input
      port completely...
      */
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processAfterTouch(unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processChannelPressure(unsigned char channel, unsigned char pressure, jack_nframes_t time);
    /**
      Reimplemented from EventProcessorClient.

      If the given event is a TimebaseEvent, the internal musical timing settings
      are changed accordingly.

      See the documentation of timebase() for further information about the behavious
      of this class when changing the musical timing settings.
      */
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
    /**
      Computes current bar, beat and tick from the current frame time, the frame rate and the beats per minute.

      Beats are computed incremental if time advances linearly. This means that if the tempo (given in bpm)
      is changed while transport is running normally, there is no discontinuity in the musical timing information.

      However, if there is a discontinuity in time (through a transport position change), the current beat is
      computed from scratch, resulting in a discontinuity also in the musical timing information.

      If the beats per bar or the beat type change there will also be discontinuities, regardless of wether
      time advances linearly or if there is a transport position change.
      */
    virtual void timebase(jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos);
private:
    JackRingBuffer<jack_position_t> ringBufferToThread;
    jack_nframes_t lastFrameTime;
    double currentBeatTime, beatsPerMinute;
    int beatsPerBar, beatType, ticksPerBeat;

    /**
      This is the JackTimebaseCallback which is registered at the Jack server.
      The implementation just casts the given void* arg pointer to a pointer
      to a JackTransportClient object and calls the timebase() method of that
      object.
      */
    static void timebase(jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos, void *arg);
};

class JackTransportThread : public JackThread {
    Q_OBJECT
public:
    JackTransportThread(JackTransportClient *client, QObject *parent = 0);

    void setRingBufferFromClient(JackRingBuffer<jack_position_t> *ringBufferFromClient);
signals:
    void changedPosition(const QString &pos);
protected:
    void processDeferred();
private:
    JackRingBuffer<jack_position_t> *ringBufferFromClient;
};

class JackTransportGraphicsItem : public GraphicsLabelItem
{
    Q_OBJECT
public:
    JackTransportGraphicsItem(QGraphicsItem *parent = 0);
public slots:
    void changePosition(const QString &pos);
private:
    JackTransportClient *client;
};


#endif // JACKTRANSPORTCLIENT_H
