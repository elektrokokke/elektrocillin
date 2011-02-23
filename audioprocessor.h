#ifndef SAMPLED_H
#define SAMPLED_H

#include <QVector>
#include <QStringList>
#include "metajack/jack.h"
#include <jack/ringbuffer.h>

class AudioProcessor
{
public:
    AudioProcessor(const QStringList &inputPortNames, const QStringList &outputPortNames, double sampleRate = 44100);
    AudioProcessor(const AudioProcessor &tocopy);

    const QStringList & getInputPortNames() const;
    const QStringList & getOutputPortNames() const;
    int getNrOfInputs() const;
    int getNrOfOutputs() const;

    virtual void setSampleRate(double sampleRate);
    double getSampleRate() const;
    double getSampleDuration() const;
    double convertHertzToRadians(double hertz) const;

    double processAudio0(jack_nframes_t time);
    double processAudio1(double input, jack_nframes_t time);
    double processAudio2(double input1, double input2, jack_nframes_t time);

    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time) = 0;

private:
    double sampleRate, sampleDuration;
    QStringList inputPortNames, outputPortNames;
    QVector<double> inputs, outputs;
};

#endif // SAMPLED_H
