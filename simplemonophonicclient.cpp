#include "simplemonophonicclient.h"
#include <cmath>
#include <jack/midiport.h>
#include <QDebug>

SimpleMonophonicClient::SimpleMonophonicClient(const QString &clientName) :
    Midi2AudioClient(clientName)
{
}

SimpleMonophonicClient::~SimpleMonophonicClient()
{
    close();
}

bool SimpleMonophonicClient::init()
{
    if (!Midi2AudioClient::init()) {
        return false;
    }
    // initialize the oscillator with the current sample rate:
    synthesizer.setSampleRate(getSampleRate());
    return true;
}

bool SimpleMonophonicClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    void *midiInputBuffer = jack_port_get_buffer(getMidiInputPort(), nframes);
    jack_default_audio_sample_t *audioOutputBuffer = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(getAudioOutputPort(), nframes));
    // interpret the midi input and create sound accordingly:
    jack_nframes_t currentFrame = 0;
    jack_nframes_t currentMidiEventIndex = 0;
    jack_nframes_t midiEventCount = jack_midi_get_event_count(midiInputBuffer);
    for (; currentFrame < nframes; ) {
        // get the next midi event, if there is any:
        if (currentMidiEventIndex < midiEventCount) {
            //qDebug() << "(currentMidiEventIndex < midiEventCount) == true";
            jack_midi_event_t midiEvent;
            jack_midi_event_get(&midiEvent, midiInputBuffer, currentMidiEventIndex);
            // produce audio until the event happens:
            for (; currentFrame < midiEvent.time; currentFrame++) {
                audioOutputBuffer[currentFrame] = synthesizer.nextSample();
            }
            currentMidiEventIndex++;
            // interpret the midi event:
            unsigned char statusByte = midiEvent.buffer[0];
            unsigned char highNibble = statusByte >> 4;
            if (highNibble == 0x08) {
                unsigned char noteNumber = midiEvent.buffer[1];
                synthesizer.popNote(noteNumber);
            } else if (highNibble == 0x09) {
                unsigned char noteNumber = midiEvent.buffer[1];
                unsigned char velocity = midiEvent.buffer[2];
                if (velocity) {
                    // note on event:
                    synthesizer.pushNote(noteNumber);
                } else {
                    // note off event:
                    synthesizer.popNote(noteNumber);
                }
            } else if (highNibble == 0x0B) {
                // control change:
                unsigned char controller = midiEvent.buffer[1];
                unsigned char value = midiEvent.buffer[2];
                synthesizer.setController(controller, value);
            } else if (highNibble == 0x0E) {
                // pitch wheel:
                unsigned char low = midiEvent.buffer[1];
                unsigned char high = midiEvent.buffer[2];
                unsigned int pitch = (high << 7) + low;
                synthesizer.setMidiPitch(pitch);
            }
        } else {
            // produce audio until the end of the buffer:
            for (; currentFrame < nframes; currentFrame++) {
                audioOutputBuffer[currentFrame] = synthesizer.nextSample();
            }
        }
    }
    return true;
}
