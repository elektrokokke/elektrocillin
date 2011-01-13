#include "oscillator.h"
#include <cmath>
#include <QtGlobal>

Oscillator::Oscillator() :
    phase(0.0),
    filter(0.01, 0.99)
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
    // the frequency is rounded to integer fractions of the sample rate to avoid jitter:
    this->frequency = getSampleRate() / (double)qRound(getSampleRate() / frequency);
    // recompute phase increment:
    computePhaseIncrement();
}

double Oscillator::getFrequency() const
{
    return frequency;
}

void Oscillator::reset()
{
    phase = 0.0;
    filter.reset();
}

double Oscillator::nextSample()
{
    double value = valueAtPhase(phase);
    // advance phase:
    phase += filter.filter(phaseIncrement);
    if (phase >= 2.0 * M_PI) {
        phase -= 2.0 * M_PI;
    }
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

