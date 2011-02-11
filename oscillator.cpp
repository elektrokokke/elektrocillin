#include "oscillator.h"
#include <cmath>

Oscillator::Oscillator(double frequencyModulationIntensity_, double sampleRate, const QStringList &additionalInputPortNames) :
    MidiProcessor(QStringList("Pitch modulation") + QStringList("Pulse width modulation") + additionalInputPortNames, QStringList("Audio out"), sampleRate),
    frequency(0),
    frequencyPitchBendFactor(1),
    frequencyModulationFactor(1),
    frequencyModulationIntensity(frequencyModulationIntensity_),
    phase(0),
    phaseIncrement(0),
    pulseWidthInterpolator(QVector<double>(), QVector<double>())
{
    pulseWidthInterpolator.getX().append(0);
    pulseWidthInterpolator.getY().append(0);
    pulseWidthInterpolator.getX().append(M_PI);
    pulseWidthInterpolator.getY().append(M_PI);
    pulseWidthInterpolator.getX().append(2 * M_PI);
    pulseWidthInterpolator.getY().append(2 * M_PI);
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
    // consider frequency modulation input:
    frequencyModulationFactor = pow(1 + frequencyModulationIntensity, inputs[0]);
    computePhaseIncrement();
    // consider pulse width modulation input:
    double pulseWidth = (std::max(std::min(inputs[1], 1.0), -1.0) + 1.0) * M_PI;
    pulseWidthInterpolator.getX()[1] = pulseWidth;
    // get previous and current phase (considering pulse width modulation):
    double previousPhaseAfterPm = pulseWidthInterpolator.evaluate(phase);
    // advance phase:
    phase += phaseIncrement;
    if (phase >= 2.0 * M_PI) {
        phase -= 2.0 * M_PI;
    }
    double phaseAfterPm = pulseWidthInterpolator.evaluate(phase);
    // compute the oscillator output:
    outputs[0] = valueAtPhase(phaseAfterPm, previousPhaseAfterPm);
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

double Oscillator::valueAtPhase(double phase, double)
{
    return sin(phase);
}

void Oscillator::computePhaseIncrement()
{
    phaseIncrement = frequency * frequencyPitchBendFactor * frequencyModulationFactor * 2.0 * M_PI / getSampleRate();
}

