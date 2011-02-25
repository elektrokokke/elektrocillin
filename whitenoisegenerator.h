#ifndef WHITENOISEGENERATOR_H
#define WHITENOISEGENERATOR_H

#include "audioprocessorclient.h"

class WhiteNoiseGenerator : public AudioProcessor
{
public:
    WhiteNoiseGenerator(double sampleRate = 44100);

    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
};

class WhiteNoiseGeneratorClient : public AudioProcessorClient
{
public:
    WhiteNoiseGeneratorClient(const QString &clientName);
    ~WhiteNoiseGeneratorClient();

    virtual JackClientFactory * getFactory();
};

#endif // WHITENOISEGENERATOR_H
