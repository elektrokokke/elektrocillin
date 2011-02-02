#ifndef SAMPLED_H
#define SAMPLED_H

#include <QVector>

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

    double processAudio0();
    double processAudio1(double input);
    virtual void processAudio(const double *inputs, double *outputs) = 0;

private:
    double sampleRate, sampleDuration;
    QVector<double> inputs, outputs;
};

#endif // SAMPLED_H
