#ifndef SAMPLED_H
#define SAMPLED_H

#include <QVector>
#include <jack/jack.h>

class AudioProcessor
{
public:
    AudioProcessor(int nrOfInputs = 0, int nrOfOutputs = 0, double sampleRate = 44100);

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
    QVector<double> inputs, outputs;
};

#endif // SAMPLED_H
