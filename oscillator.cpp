#include "oscillator.h"
#include <cmath>

Oscillator::Oscillator()
{
}

void Oscillator::setSampleRate(double sampleRate)
{
    this->sampleRate = sampleRate;
    // recompute phase increment:
    if (!frequencies.isEmpty()) {
        computePhaseIncrement();
    }
}

void Oscillator::pushNote(unsigned char midiNoteNumber)
{
    // reset phase if this is the first frequency on the stack:
    if (midiNoteNumbers.isEmpty()) {
        phase = 0.0;
    }
    midiNoteNumbers.push(midiNoteNumber);
    frequencies.push(computeFrequencyFromMidiNoteNumber(midiNoteNumber));
    computePhaseIncrement();
}

void Oscillator::popNote(unsigned char midiNoteNumber)
{
    if (!midiNoteNumbers.isEmpty()) {
        // test if this is the topmost note:
        if (midiNoteNumbers.top() == midiNoteNumber) {
            midiNoteNumbers.pop();
            frequencies.pop();
            if (!frequencies.isEmpty()) {
                computePhaseIncrement();
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

double Oscillator::createSample()
{
    if (!frequencies.isEmpty()) {
        double value = valueAtPhase(phase);
        // advance phase:
        phase += phaseIncrement;
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
        return value;
    } else {
        // no frequency is given, be silent:
        return 0.0;
    }
}

double Oscillator::valueAtPhase(double phase)
{
    return sin(phase);
}

void Oscillator::computePhaseIncrement()
{
    phaseIncrement = frequencies.top() * 2.0 * M_PI / sampleRate;
}

double Oscillator::computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber)
{
    // 440 Hz is note number 69:
    double octave = ((double)midiNoteNumber - 69.0) / 12.0;
    double frequency = 440.0 * pow(2.0, octave);
    return frequency;
}
