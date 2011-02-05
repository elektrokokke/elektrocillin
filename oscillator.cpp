#include "oscillator.h"
#include <cmath>

Oscillator::Oscillator(double frequencyModulationIntensity_, double sampleRate, const QStringList &additionalInputPortNames) :
    MidiProcessor(QStringList("pitch_modulation_in") + additionalInputPortNames, QStringList("oscillator_out"), sampleRate),
    frequency(0),
    frequencyPitchBendFactor(1),
    frequencyModulationFactor(1),
    frequencyModulationIntensity(frequencyModulationIntensity_),
    phase(0),
    phaseIncrement(0)
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
    frequency = computeFrequencyFromMidiNoteNumber(noteNumber);
    computePhaseIncrement();
}

void Oscillator::processPitchBend(unsigned char, unsigned int value, jack_nframes_t)
{
    frequencyPitchBendFactor = computePitchBendFactorFromMidiPitch(value);
    computePhaseIncrement();
}

void Oscillator::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    frequencyModulationFactor = pow(1 + frequencyModulationIntensity, inputs[0]);
    computePhaseIncrement();
    outputs[0] = valueAtPhase(phase);
    // advance phase:
    phase += phaseIncrement;
    if (phase >= 2.0 * M_PI) {
        phase -= 2.0 * M_PI;
    }
}

void Oscillator::setFrequency(double hertz)
{
    frequency = hertz;
    computePhaseIncrement();
}

double Oscillator::getFrequency() const
{
    return frequency;
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
    phaseIncrement = frequency * frequencyPitchBendFactor * frequencyModulationFactor * 2.0 * M_PI / getSampleRate();
}

