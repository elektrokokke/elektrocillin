#include "monophonicsynthesizer.h"
#include <cmath>

MonophonicSynthesizer::MonophonicSynthesizer(double sampleRate) :
    MidiProcessor(0, 1, sampleRate),
    morph(0.0),
    osc1(M_PI),
    osc2(0.1),
    morphOsc1(&osc1, &osc2),
    envelope(0.001, 0.01, 0.25, 0.5),
    filterAudio(22050),
    filterMorph(44.1)
{
    morphOsc1.setMorph(0.5);
    setSampleRate(sampleRate);
}

void MonophonicSynthesizer::setSampleRate(double sampleRate)
{
    MidiProcessor::setSampleRate(sampleRate);
    osc1.setSampleRate(sampleRate);
    osc2.setSampleRate(sampleRate);
    morphOsc1.setSampleRate(sampleRate);
    envelope.setSampleRate(sampleRate);
    filterAudio.setSampleRate(sampleRate);
    filterMorph.setSampleRate(sampleRate);
}

void MonophonicSynthesizer::processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity)
{
    midiNoteNumbers.push(noteNumber);
    morphOsc1.processNoteOn(channel, noteNumber, velocity);
    // (re-)trigger the ADSR envelope:
    envelope.processNoteOn(channel, noteNumber, velocity);
}

void MonophonicSynthesizer::processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity)
{
    if (!midiNoteNumbers.isEmpty()) {
        // test if this is the topmost note:
        if (midiNoteNumbers.top() == noteNumber) {
            midiNoteNumbers.pop();
            if (midiNoteNumbers.isEmpty()) {
                morphOsc1.processNoteOff(channel, noteNumber, velocity);
                // enter the release phase:
                envelope.processNoteOff(channel, noteNumber, velocity);
            } else {
                morphOsc1.processNoteOn(channel, noteNumber, velocity);
                // retrigger the ADSR envelope:
                envelope.processNoteOn(channel, noteNumber, velocity);
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

void MonophonicSynthesizer::processPitchBend(unsigned char channel, unsigned int pitch)
{
    morphOsc1.processPitchBend(channel, pitch);
}

void MonophonicSynthesizer::processController(unsigned char, unsigned char value)
{
    morph = (double)value / 127.0;
}

void MonophonicSynthesizer::processAudio(const double *, double *outputs)
{
    // get level from ADSR envelope:
    double envelopeLevel = envelope.processAudio0();
    morphOsc1.setMorph(filterMorph.processAudio1(morph));
    double oscillatorLevel = morphOsc1.processAudio0();
    outputs[0] = filterAudio.processAudio1(envelopeLevel * oscillatorLevel);
}
