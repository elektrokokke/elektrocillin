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

      See AudioProcessorClient::AudioProcessorClient(const QString &, const QStringList &, const QStringList &)
      for a description of the parameters.
      */
    MidiProcessorClient(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames);

    void deactivateMidiInput();

    virtual bool init();
    virtual bool process(jack_nframes_t nframes);
    virtual void processMidi(jack_nframes_t start, jack_nframes_t end);
    virtual void processMidi(const MidiEvent &event, jack_nframes_t time);

    // Override these in subclasses:
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);

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
    jack_nframes_t midiEventCount;
    bool midiInput;
};

#endif // NOTETRIGGEREDCLIENT_H
