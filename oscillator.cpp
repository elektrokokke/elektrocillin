#include "oscillator.h"
#include <cmath>
#include <QDebug>

Oscillator::Oscillator() :
    frequency(0.0),
    phase(0.0),
    phaseIncrement(0.0),
    sampleNr(0)
{
}

void Oscillator::setSampleRate(double sampleRate)
{
    AudioSource::setSampleRate(sampleRate);
    // recompute phase increment:
    computePhaseIncrement();
}

void Oscillator::setFrequency(double frequency)
{
    this->frequency = frequency;
    // recompute phase increment:
    computePhaseIncrement();
}

double Oscillator::getPhaseIncrement() const
{
    return phaseIncrement;
}

double Oscillator::getFrequency() const
{
    return frequency;
}

void Oscillator::reset()
{
    phase = 0.0;
}

double Oscillator::nextSample()
{
    double value = valueAtPhase(phase);
    // advance phase:
    phase += phaseIncrement;
    if (phase >= 2.0 * M_PI) {
        phase -= 2.0 * M_PI;
    }
    sampleNr++;
    return value;
}

double Oscillator::valueAtPhase(double phase)
{
    return sin(phase);
}

void Oscillator::computePhaseIncrement()
{
    phaseIncrement = frequency * 2.0 * M_PI / getSampleRate();
}

