#include "monophonicsynthesizer.h"
#include <cmath>

MonophonicSynthesizer::MonophonicSynthesizer(double sampleRate) :
    NoteTriggered(0, 1, sampleRate),
//    morph(0.0),
    osc1(M_PI),
//    osc2(0.1),
//    morphOsc1(&osc1, &osc2),
    envelope(0.01, 0.01, 0.75, 0.5)
//    filterAudio(0.5),
//    filterMorph(0.001)
{
//    morphOsc1.setMorph(0.5);
    setSampleRate(sampleRate);
}

void MonophonicSynthesizer::setSampleRate(double sampleRate)
{
    NoteTriggered::setSampleRate(sampleRate);
    osc1.setSampleRate(sampleRate);
//    osc2.setSampleRate(sampleRate);
//    morphOsc1.setSampleRate(sampleRate);
    envelope.setSampleRate(sampleRate);
}

void MonophonicSynthesizer::noteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity)
{
    midiNoteNumbers.push(noteNumber);
    osc1.noteOn(channel, noteNumber, velocity);
    // (re-)trigger the ADSR envelope:
    envelope.noteOn(channel, noteNumber, velocity);
}

void MonophonicSynthesizer::noteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity)
{
    if (!midiNoteNumbers.isEmpty()) {
        // test if this is the topmost note:
        if (midiNoteNumbers.top() == noteNumber) {
            midiNoteNumbers.pop();
            if (midiNoteNumbers.isEmpty()) {
                osc1.noteOff(channel, noteNumber, velocity);
                // enter the release phase:
                envelope.noteOff(channel, noteNumber, velocity);
            } else {
                osc1.noteOn(channel, noteNumber, velocity);
                // retrigger the ADSR envelope:
                envelope.noteOn(channel, noteNumber, velocity);
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

void MonophonicSynthesizer::pitchBend(unsigned char channel, unsigned int pitch)
{
    osc1.pitchBend(channel, pitch);
}

void MonophonicSynthesizer::controller(unsigned char, unsigned char value)
{
    morph = (double)value / 127.0;
}

void MonophonicSynthesizer::process(const double *inputs, double *outputs)
{
    // get level from ADSR envelope:
//    double envelopeLevel = envelope.nextSample();
//    morphOsc1.setMorph(filterMorph.filter(morph));
//    double oscillatorLevel = morphOsc1.nextSample();
//    return filterAudio.filter(envelopeLevel * oscillatorLevel);
    outputs[0] = envelope.process0() * osc1.process0();
}
