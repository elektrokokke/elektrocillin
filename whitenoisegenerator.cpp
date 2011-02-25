#include "whitenoisegenerator.h"
#include <cstdlib>

WhiteNoiseGenerator::WhiteNoiseGenerator(double sampleRate) :
    AudioProcessor(QStringList(), QStringList("Noise out"), sampleRate)
{
}

void WhiteNoiseGenerator::processAudio(const double *, double *outputs, jack_nframes_t)
{
    int randomNumber = rand();
    outputs[0] = (double)randomNumber / (double)RAND_MAX * 2.0 - 1.0;
}

WhiteNoiseGeneratorClient::WhiteNoiseGeneratorClient(const QString &clientName) :
    AudioProcessorClient(clientName, new WhiteNoiseGenerator(44100))
{
}

WhiteNoiseGeneratorClient::~WhiteNoiseGeneratorClient()
{
    delete getAudioProcessor();
}

class WhiteNoiseGeneratorClientFactory : public JackClientFactory
{
public:
    WhiteNoiseGeneratorClientFactory()
    {
        JackClientFactory::registerFactory(this);
    }
    QString getName()
    {
        return "White noise generator";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new WhiteNoiseGeneratorClient(clientName);
    }
    static WhiteNoiseGeneratorClientFactory factory;
};

WhiteNoiseGeneratorClientFactory WhiteNoiseGeneratorClientFactory::factory;

JackClientFactory * WhiteNoiseGeneratorClient::getFactory()
{
    return &WhiteNoiseGeneratorClientFactory::factory;
}
