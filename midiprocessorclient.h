#ifndef NOTETRIGGEREDCLIENT_H
#define NOTETRIGGEREDCLIENT_H

#include "audioprocessorclient.h"
#include "midiprocessor.h"

class MidiProcessorClient : public AudioProcessorClient
{
public:
    MidiProcessorClient(const QString &clientName, MidiProcessor *midiProcessor);
    virtual ~MidiProcessorClient();

protected:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);
    virtual void processMidi(const jack_midi_event_t &event);

    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value);
    virtual void processPitchBend(unsigned char channel, unsigned int value);

private:
    MidiProcessor *midiProcessor;
    jack_port_t * midiInputPort;
};

#endif // NOTETRIGGEREDCLIENT_H
