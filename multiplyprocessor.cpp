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
    close();
    delete getAudioProcessor();
}

MultiplyProcessor * MultiplyClient::getMultiplyProcessor()
{
    return (MultiplyProcessor*)getAudioProcessor();
}

void MultiplyClient::saveState(QDataStream &stream)
{
    stream << getMultiplyProcessor()->getGainFactor();
}

void MultiplyClient::loadState(QDataStream &stream)
{
    double gain;
    stream >> gain;
    getMultiplyProcessor()->setGainFactor(gain);
}

class MultiplyClientFactory : public JackClientFactory
{
public:
    MultiplyClientFactory()
    {
        JackClientFactory::registerFactory(this);
    }
    QString getName()
    {
        return "Multiplier";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new MultiplyClient(clientName);
    }
    static MultiplyClientFactory factory;
};

MultiplyClientFactory MultiplyClientFactory::factory;

JackClientFactory * MultiplyClient::getFactory()
{
    return &MultiplyClientFactory::factory;
}
