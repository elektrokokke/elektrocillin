#include "oscillator.h"
#include <cmath>
#include <QtGlobal>

Oscillator::Oscillator(double frequencyModulationIntensity_, double sampleRate, const QStringList &additionalInputPortNames) :
    MidiProcessor(QStringList("Pitch modulation") + additionalInputPortNames, QStringList("Audio out"), sampleRate),
    frequency(441),
    frequencyPitchBendFactor(1),
    frequencyModulationFactor(1),
    frequencyModulationIntensity(frequencyModulationIntensity_),
    phase(0),
    normalizedAngularFrequency(0)
{
}

void Oscillator::setSampleRate(double sampleRate)
{
    MidiProcessor::setSampleRate(sampleRate);
    // recompute phase increment:
    computeNormalizedAngularFrequency();
}

void Oscillator::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t)
{
    frequency = computeFrequencyFromMidiNoteNumber(noteNumber);
    computeNormalizedAngularFrequency();
}

void Oscillator::processPitchBend(unsigned char, unsigned int value, jack_nframes_t)
{
    frequencyPitchBendFactor = computePitchBendFactorFromMidiPitch(value);
    computeNormalizedAngularFrequency();
}

void Oscillator::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    // consider frequency modulation input:
    frequencyModulationFactor = pow(1 + frequencyModulationIntensity, inputs[0]);
    computeNormalizedAngularFrequency();
    double phase2 = phase + normalizedAngularFrequency;
    if (phase2 >= 2.0 * M_PI) {
        phase2 -= 2.0 * M_PI;
    }
    // compute the oscillator output:
    //outputs[0] = valueAtPhase(0.5 * (phase + phase2));
    outputs[0] = valueAtPhase(phase);
    phase = phase2;
}

void Oscillator::setFrequency(double hertz)
{
    frequency = hertz;
    computeNormalizedAngularFrequency();
}

double Oscillator::getFrequency() const
{
    return frequency;
}

double Oscillator::getNormalizedAngularFrequency() const
{
    return normalizedAngularFrequency;
}

double Oscillator::valueAtPhase(double phase)
{
    return sin(phase);
}

void Oscillator::computeNormalizedAngularFrequency()
{
    normalizedAngularFrequency = 2.0 * M_PI * frequency * frequencyPitchBendFactor * frequencyModulationFactor / getSampleRate();
}

