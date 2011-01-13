#include "monophonicsynthesizer.h"
#include <cmath>

MonophonicSynthesizer::MonophonicSynthesizer() :
    envelope(0.001, 0.01, 0.5, 0.5)
{
}

void MonophonicSynthesizer::pushNote(unsigned char midiNoteNumber)
{
    midiNoteNumbers.push(midiNoteNumber);
    frequencies.push(computeFrequencyFromMidiNoteNumber(midiNoteNumber));
    oscillator.setFrequency(frequencies.top());
    // (re-)trigger the ADSR envelope:
    envelope.noteOn();
}

void MonophonicSynthesizer::popNote(unsigned char midiNoteNumber)
{
    if (!midiNoteNumbers.isEmpty()) {
        // test if this is the topmost note:
        if (midiNoteNumbers.top() == midiNoteNumber) {
            midiNoteNumbers.pop();
            frequencies.pop();
            if (midiNoteNumbers.isEmpty()) {
                // enter the release phase:
                envelope.noteOff();
            } else {
                oscillator.setFrequency(frequencies.top());
                // retrigger the ADSR envelope:
                envelope.noteOn();
            }
        } else {
            // the note has to be removed from somewhere in the stack
            // (current playing note remains unchanged):
            int index = midiNoteNumbers.lastIndexOf(midiNoteNumber);
            if (index != -1) {
                midiNoteNumbers.remove(index);
                frequencies.remove(index);
            }
        }
    }
}

double MonophonicSynthesizer::nextSample()
{
    // get level from ADSR envelope:
    double envelopeLevel = envelope.nextSample();
    if (envelopeLevel == 0.0) {
        oscillator.reset();
    }
    double oscillatorLevel = oscillator.nextSample();
    return filter.filter(envelopeLevel * oscillatorLevel);
}

double MonophonicSynthesizer::computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber)
{
    // 440 Hz is note number 69:
    double octave = ((double)midiNoteNumber - 69.0) / 12.0;
    double frequency = 440.0 * pow(2.0, octave);
    return frequency;
}
