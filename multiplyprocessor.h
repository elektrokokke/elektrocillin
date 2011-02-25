#ifndef MULTIPLYPROCESSOR_H
#define MULTIPLYPROCESSOR_H

#include "audioprocessorclient.h"

class MultiplyProcessor : public AudioProcessor
{
public:
    MultiplyProcessor(double gain = 1.0, double sampleRate = 44100);
    MultiplyProcessor(const QStringList &inputPortNames, double gain = 1.0, double sampleRate = 44100);

    void setGainFactor(double gain);
    double getGainFactor() const;

    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

private:
    double gain;
};

class MultiplyClient : public AudioProcessorClient
{
public:
    MultiplyClient(const QString &clientName);
    ~MultiplyClient();

    virtual JackClientFactory * getFactory();
};

#endif // MULTIPLYPROCESSOR_H
