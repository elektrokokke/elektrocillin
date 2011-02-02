#ifndef SAMPLED_H
#define SAMPLED_H

#include <QVector>

class Sampled
{
public:
    Sampled(int nrOfInputs = 0, int nrOfOutputs = 0, double sampleRate = 44100);

    int getNrOfInputs() const;
    int getNrOfOutputs() const;

    virtual void setSampleRate(double sampleRate);
    double getSampleRate() const;
    double getSampleDuration() const;
    double getFrequencyInRadians(double hertz) const;

    double process0();
    double process1(double input);
    virtual void process(const double *inputs, double *outputs) = 0;

private:
    double sampleRate, sampleDuration;
    QVector<double> inputs, outputs;
};

#endif // SAMPLED_H
