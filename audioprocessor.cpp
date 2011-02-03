#include "audioprocessor.h"
#include <cmath>

AudioProcessor::AudioProcessor(int nrOfInputs, int nrOfOutputs, double sampleRate_) :
    sampleRate(sampleRate_),
    sampleDuration(1.0 / sampleRate),
    inputs(nrOfInputs),
    outputs(nrOfOutputs)
{
}

int AudioProcessor::getNrOfInputs() const
{
    return inputs.size();
}

int AudioProcessor::getNrOfOutputs() const
{
    return outputs.size();
}

void AudioProcessor::setSampleRate(double sampleRate)
{
    this->sampleRate = sampleRate;
    sampleDuration = 1.0 / sampleRate;
}

double AudioProcessor::getSampleRate() const
{
    return sampleRate;
}

double AudioProcessor::getSampleDuration() const
{
    return sampleDuration;
}

double AudioProcessor::convertHertzToRadians(double hertz) const
{
    return hertz * 2.0 * M_PI / getSampleRate();
}

double AudioProcessor::processAudio0()
{
    Q_ASSERT((inputs.size() == 0) && (outputs.size() == 1));
    processAudio(inputs.data(), outputs.data());
    return outputs[0];
}

double AudioProcessor::processAudio1(double input)
{
    Q_ASSERT((inputs.size() == 1) && (outputs.size() == 1));
    inputs[0] = input;
    processAudio(inputs.data(), outputs.data());
    return outputs[0];
}

double AudioProcessor::processAudio2(double input1, double input2)
{
    Q_ASSERT((inputs.size() == 2) && (outputs.size() == 1));
    inputs[0] = input1;
    inputs[1] = input2;
    processAudio(inputs.data(), outputs.data());
    return outputs[0];
}
