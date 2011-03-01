#ifndef JACKTHREADEVENTPROCESSOR_H
#define JACKTHREADEVENTPROCESSOR_H

#include "eventprocessorclient.h"
#include "jackthread.h"

class JackThreadEventProcessorClient : public EventProcessorClient2
{
public:
    JackThreadEventProcessorClient(JackThread *thread, const QString &clientName, MidiProcessor *midiProcessor, size_t ringBufferSize = 1024) :
        EventProcessorClient2(clientName, midiProcessor, ringBufferSize),
        jackThread(thread)
    {}
    /**
      Constructor for subclasses that do not want to use a MidiProcessor,
      but reimplement the respective methods such as to do the MidiProcessor's
      work themselves.
      Methods to reimplement:
      - processAudio(const double*, double*, jack_nframes_t) OR
      - processAudio(jack_nframes_t, jack_nframes_t),
      - processNoteOn AND
      - processNoteOff AND
      - processController AND
      - processPitchBend OR
      - processMidi(const MidiEvent &, jack_nframes_t) OR
      - processMidi(jack_nframes_t, jack_nframes_t)
      */
    JackThreadEventProcessorClient(JackThread *thread, const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, size_t ringBufferSize = 1024) :
        EventProcessorClient2(clientName, inputPortNames, outputPortNames, ringBufferSize),
        jackThread(thread)
    {}

    JackThread * getJackThread()
    {
        return jackThread;
    }
    void wakeJackThread()
    {
        jackThread->wake();
    }

protected:
    /**
      This starts the associated JackThread.
      You must call this in your reimplementation, but only AFTER initializing any ring buffers
      the JackThread might use (you could of course also initialize ring buffers in the
      constructor instead).
      */
    virtual bool init()
    {
        jackThread->start();
        return EventProcessorClient2::init();
    }
    /**
      This stops the associated JackThread.
      You must call this in your reimplementation, but only BEFORE freeing any ring buffers
      the JackThread might use.
      */
    virtual void deinit()
    {
        jackThread->stop();
        EventProcessorClient2::deinit();
    }

private:
    JackThread *jackThread;

};

#endif // JACKTHREADEVENTPROCESSOR_H
