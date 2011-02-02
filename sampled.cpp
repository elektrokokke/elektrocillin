#include "sampled.h"
#include <cmath>

Sampled::Sampled(int nrOfInputs, int nrOfOutputs, double sampleRate_) :
    sampleRate(sampleRate_),
    sampleDuration(1.0 / sampleRate),
    inputs(nrOfInputs),
    outputs(nrOfOutputs)
{
}

int Sampled::getNrOfInputs() const
{
    return inputs.size();
}

int Sampled::getNrOfOutputs() const
{
    return outputs.size();
}

void Sampled::setSampleRate(double sampleRate)
{
    this->sampleRate = sampleRate;
    sampleDuration = 1.0 / sampleRate;
}

double Sampled::getSampleRate() const
{
    return sampleRate;
}

double Sampled::getSampleDuration() const
{
    return sampleDuration;
}

double Sampled::getFrequencyInRadians(double hertz) const
{
    return hertz * 2.0 * M_PI / getSampleRate();
}

double Sampled::process0()
{
    Q_ASSERT((inputs.size() == 0) && (outputs.size() == 1));
    process(inputs.data(), outputs.data());
    return outputs[0];
}

double Sampled::process1(double input)
{
    Q_ASSERT((inputs.size() == 1) && (outputs.size() == 1));
    inputs[0] = input;
    process(inputs.data(), outputs.data());
    return outputs[0];
}
