#ifndef NOTETRIGGEREDCLIENT_H
#define NOTETRIGGEREDCLIENT_H

#include "audioprocessorclient.h"
#include "midiprocessor.h"
#include <jack/midiport.h>

class MidiProcessorClient : public AudioProcessorClient
{
public:
    struct MidiEvent {
        size_t size;
        jack_midi_data_t buffer[3];
    };

    MidiProcessorClient(const QString &clientName, MidiProcessor *midiProcessor);
    virtual ~MidiProcessorClient();

    MidiProcessor * getMidiProcessor();

protected:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);
    virtual void processMidi(jack_nframes_t start, jack_nframes_t end);
    virtual void processMidi(const MidiEvent &event);

    // Override these in subclasses:
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value);
    virtual void processPitchBend(unsigned char channel, unsigned int value);

    /**
      If you override process(jack_nframes_t nframes) call this function
      before you call processMidi(jack_nframes_t start, jack_nframes_t end).
      */
    void getMidiPortBuffer(jack_nframes_t nframes);

private:
    MidiProcessor *midiProcessor;
    jack_port_t * midiInputPort;
    void *midiInputBuffer;
    jack_nframes_t currentMidiEventIndex;
    jack_nframes_t midiEventCount ;
};

#endif // NOTETRIGGEREDCLIENT_H
