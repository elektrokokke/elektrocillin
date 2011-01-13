#include "monophonicsynthesizer.h"
#include <cmath>

MonophonicSynthesizer::MonophonicSynthesizer() :
    attack(0), release(0)
{
}

void MonophonicSynthesizer::setSampleRate(double sampleRate)
{
    oscillator.setSampleRate(sampleRate);
}

double MonophonicSynthesizer::getSampleRate() const
{
    return oscillator.getSampleRate();
}

void MonophonicSynthesizer::pushNote(unsigned char midiNoteNumber)
{
    if (midiNoteNumbers.isEmpty()) {
        attack = 99 - release;
        release = 0;
    }
    midiNoteNumbers.push(midiNoteNumber);
    frequencies.push(computeFrequencyFromMidiNoteNumber(midiNoteNumber));
    oscillator.setFrequency(frequencies.top());
}

void MonophonicSynthesizer::popNote(unsigned char midiNoteNumber)
{
    if (!midiNoteNumbers.isEmpty()) {
        // test if this is the topmost note:
        if (midiNoteNumbers.top() == midiNoteNumber) {
            midiNoteNumbers.pop();
            frequencies.pop();
            if (midiNoteNumbers.isEmpty()) {
                release = 99 - attack;
                attack = 0;
            } else {
                oscillator.setFrequency(frequencies.top());
            }
        } else {
            // the note has to be removed from somewhere in the stack:
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
    // incorporate very short attack and release phase to avoid clicks:
    if (attack) {
        double sample = oscillator.nextSample() * (1.0 - 0.01 * (double)attack);
        attack -= 1;
        return sample;
    } else if (release) {
        double sample = oscillator.nextSample() * 0.01 * (double)release;
        release -= 1;
        return sample;
    } else if (!midiNoteNumbers.isEmpty()) {
        return oscillator.nextSample();
    } else {
        return 0.0;
    }
}

double MonophonicSynthesizer::computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber)
{
    // 440 Hz is note number 69:
    double octave = ((double)midiNoteNumber - 69.0) / 12.0;
    double frequency = 440.0 * pow(2.0, octave);
    return frequency;
}
