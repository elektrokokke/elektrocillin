#include "simplemonophonicclient.h"
#include <cmath>
#include <jack/midiport.h>
#include <QDebug>

SimpleMonophonicClient::SimpleMonophonicClient(const QString &clientName) :
    Midi2AudioClient(clientName)
{
}

bool SimpleMonophonicClient::setup()
{
    if (!Midi2AudioClient::setup()) {
        return false;
    }
    // initialize the oscillator with the current sample rate:
    oscillator.setSampleRate(getSampleRate());
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
    for (; currentFrame < nframes;) {
        // get the next midi event, if there is any:
        if (currentMidiEventIndex < midiEventCount) {
            jack_midi_event_t midiEvent;
            jack_midi_event_get(&midiEvent, midiInputBuffer, currentMidiEventIndex);
            // produce audio until the event happens:
            for (; currentFrame < midiEvent.time; currentFrame++) {
                audioOutputBuffer[currentFrame] = oscillator.createSample();
            }
            currentMidiEventIndex++;
            // interpret the midi event:
            unsigned char statusByte = midiEvent.buffer[0];
            unsigned char highNibble = statusByte >> 4;
            qDebug() << "highNibble" << highNibble;
            if (highNibble == 0x08) {
                unsigned char noteNumber = midiEvent.buffer[1];
                qDebug() << "noteNumber" << noteNumber;
                oscillator.popNote(noteNumber);
            } else if (highNibble == 0x09) {
                unsigned char noteNumber = midiEvent.buffer[1];
                qDebug() << "noteNumber" << noteNumber;
                unsigned char velocity = midiEvent.buffer[2];
                qDebug() << "velocity" << velocity;
                if (velocity) {
                    // note on event:
                    oscillator.pushNote(noteNumber);
                } else {
                    // note off event:
                    oscillator.popNote(noteNumber);
                }
            }
        } else {
            // produce audio until the end of the buffer:
            for (; currentFrame < nframes; currentFrame++) {
                audioOutputBuffer[currentFrame] = oscillator.createSample();
            }
        }
    }
    return true;
}
