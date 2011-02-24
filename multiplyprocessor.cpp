#include "multiplyprocessor.h"

MultiplyProcessor::MultiplyProcessor(double gain_, double sampleRate) :
    AudioProcessor(QStringList("Factor 1") + QStringList("Factor 2"), QStringList("Product out"), sampleRate),
    gain(gain_)
{
}

MultiplyProcessor::MultiplyProcessor(const QStringList &inputPortNames, double gain_, double sampleRate) :
    AudioProcessor(inputPortNames, QStringList("Product"), sampleRate),
    gain(gain_)
{
}

void MultiplyProcessor::setGainFactor(double gain)
{
    this->gain = gain;
}

double MultiplyProcessor::getGainFactor() const
{
    return gain;
}

void MultiplyProcessor::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    // multiply all inputs:
    double product = gain;
    for (int i = 0; i < getNrOfInputs(); i++) {
        product *= inputs[i];
    }
    outputs[0] = product;
}

MultiplyClient::MultiplyClient(const QString &clientName) :
    AudioProcessorClient(clientName, new MultiplyProcessor())
{
}

MultiplyClient::~MultiplyClient()
{
    delete getAudioProcessor();
}
