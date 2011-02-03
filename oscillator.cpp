#include "oscillator.h"
#include <cmath>

Oscillator::Oscillator(double sampleRate) :
    MidiProcessor(0, 1, sampleRate),
    noteNumber(0),
    pitchBendValue(0),
    frequencyInHertz(0.0),
    phase(0.0),
    phaseIncrement(0.0)
{
}

void Oscillator::setSampleRate(double sampleRate)
{
    MidiProcessor::setSampleRate(sampleRate);
    // recompute phase increment:
    computePhaseIncrement();
}

void Oscillator::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t)
{
    this->noteNumber = noteNumber;
    frequencyInHertz = computeFrequencyFromMidiNoteNumber(noteNumber) * computePitchBendFactorFromMidiPitch(pitchBendValue);
    computePhaseIncrement();
}

void Oscillator::processPitchBend(unsigned char, unsigned int value, jack_nframes_t)
{
    pitchBendValue = value;
    frequencyInHertz = computeFrequencyFromMidiNoteNumber(noteNumber) * computePitchBendFactorFromMidiPitch(pitchBendValue);
    computePhaseIncrement();
}

void Oscillator::processAudio(const double *, double *outputs, jack_nframes_t)
{
    outputs[0] = valueAtPhase(phase);
    // advance phase:
    phase += phaseIncrement;
    if (phase >= 2.0 * M_PI) {
        phase -= 2.0 * M_PI;
    }
}

void Oscillator::setFrequency(double hertz)
{
    frequencyInHertz = hertz;
    computePhaseIncrement();
}

double Oscillator::getFrequency() const
{
    return frequencyInHertz;
}

double Oscillator::getPhaseIncrement() const
{
    return phaseIncrement;
}

double Oscillator::valueAtPhase(double phase)
{
    return sin(phase);
}

void Oscillator::computePhaseIncrement()
{
    phaseIncrement = frequencyInHertz * 2.0 * M_PI / getSampleRate();
}

