#include "monophonicsynthesizer.h"
#include <cmath>

MonophonicSynthesizer::MonophonicSynthesizer(double sampleRate) :
    MidiProcessor(QStringList("Modulation"), QStringList("Audio out"), sampleRate),
    controller(0.0),
    envelope(0.001, 0.2, 0.2, 0.3),
    filterAudio(22050),
    filterController(44.1)
{
    setSampleRate(sampleRate);
}

void MonophonicSynthesizer::setSampleRate(double sampleRate)
{
    MidiProcessor::setSampleRate(sampleRate);
    osc.setSampleRate(sampleRate);
    envelope.setSampleRate(sampleRate);
    filterAudio.setSampleRate(sampleRate);
    filterController.setSampleRate(sampleRate);
}

void MonophonicSynthesizer::processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
    midiNoteNumbers.push(noteNumber);
    osc.processNoteOn(channel, noteNumber, velocity, time);
    // (re-)trigger the ADSR envelope:
    envelope.processNoteOn(channel, noteNumber, velocity, time);
}

void MonophonicSynthesizer::processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
    if (!midiNoteNumbers.isEmpty()) {
        // test if this is the topmost note:
        if (midiNoteNumbers.top() == noteNumber) {
            midiNoteNumbers.pop();
            if (midiNoteNumbers.isEmpty()) {
                osc.processNoteOff(channel, noteNumber, velocity, time);
                // enter the release phase:
                envelope.processNoteOff(channel, noteNumber, velocity, time);
            } else {
                osc.processNoteOn(channel, midiNoteNumbers.top(), velocity, time);
                // retrigger the ADSR envelope:
                envelope.processNoteOn(channel, midiNoteNumbers.top(), velocity, time);
            }
        } else {
            // the note has to be removed from somewhere in the stack
            // (current playing note remains unchanged):
            int index = midiNoteNumbers.lastIndexOf(noteNumber);
            if (index != -1) {
                midiNoteNumbers.remove(index);
            }
        }
    }
}

void MonophonicSynthesizer::processPitchBend(unsigned char channel, unsigned int pitch, jack_nframes_t time)
{
    osc.processPitchBend(channel, pitch, time);
}

void MonophonicSynthesizer::processController(unsigned char, unsigned char value, jack_nframes_t)
{
    controller = (double)value / 127.0;
}

void MonophonicSynthesizer::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    // get level from ADSR envelope:
    double envelopeLevel = envelope.processAudio0(time);
    filterController.processAudio1(controller, time);
    //double oscillatorLevel = osc.processAudio1(envelopeLevel + inputs[0], time);
    double oscillatorLevel = osc.processAudio2(0, inputs[0], time);
    outputs[0] = filterAudio.processAudio1(envelopeLevel * oscillatorLevel, time);
}
