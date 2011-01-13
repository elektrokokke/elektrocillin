#include "oscillator.h"
#include <cmath>
#include <QtGlobal>

Oscillator::Oscillator() :
    phase(0.0)
{
}

void Oscillator::setSampleRate(double sampleRate)
{
    this->sampleRate = sampleRate;
    // recompute phase increment:
    computePhaseIncrement();
}

double Oscillator::getSampleRate() const
{
    return sampleRate;
}

void Oscillator::setFrequency(double frequency)
{
    // the frequency is rounded to integer fractions of the sample rate to avoid jitter:
    this->frequency = sampleRate / (double)qRound(sampleRate / frequency);
    // recompute phase increment:
    computePhaseIncrement();
}

double Oscillator::getFrequency() const
{
    return frequency;
}

void Oscillator::resetPhase()
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
    return value;
}

double Oscillator::valueAtPhase(double phase)
{
    return sin(phase);
}

void Oscillator::computePhaseIncrement()
{
    phaseIncrement = frequency * 2.0 * M_PI / sampleRate;
}

