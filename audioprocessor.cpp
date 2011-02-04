#include "audioprocessor.h"
#include <cmath>

AudioProcessor::AudioProcessor(const QStringList &inputPortNames_, const QStringList &outputPortNames_, double sampleRate_) :
    sampleRate(sampleRate_),
    sampleDuration(1.0 / sampleRate),
    inputPortNames(inputPortNames_),
    outputPortNames(outputPortNames_),
    inputs(inputPortNames.size()),
    outputs(outputPortNames.size())
{
}

const QStringList & AudioProcessor::getInputPortNames() const
{
    return inputPortNames;
}
const QStringList & AudioProcessor::getOutputPortNames() const
{
    return outputPortNames;
}

int AudioProcessor::getNrOfInputs() const
{
    return inputPortNames.size();
}

int AudioProcessor::getNrOfOutputs() const
{
    return outputPortNames.size();
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

double AudioProcessor::processAudio0(jack_nframes_t time)
{
    Q_ASSERT((inputs.size() == 0) && (outputs.size() == 1));
    processAudio(inputs.data(), outputs.data(), time);
    return outputs[0];
}

double AudioProcessor::processAudio1(double input, jack_nframes_t time)
{
    Q_ASSERT((inputs.size() == 1) && (outputs.size() == 1));
    inputs[0] = input;
    processAudio(inputs.data(), outputs.data(), time);
    return outputs[0];
}

double AudioProcessor::processAudio2(double input1, double input2, jack_nframes_t time)
{
    Q_ASSERT((inputs.size() == 2) && (outputs.size() == 1));
    inputs[0] = input1;
    inputs[1] = input2;
    processAudio(inputs.data(), outputs.data(), time);
    return outputs[0];
}
