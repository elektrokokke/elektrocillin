#ifndef OSCILLATOR_H
#define OSCILLATOR_H

class Oscillator
{
public:
    Oscillator();

    void setSampleRate(double sampleRate);
    double getSampleRate() const;
    void setFrequency(double frequency);
    double getFrequency() const;

    void resetPhase();

    double nextSample();

protected:
    virtual double valueAtPhase(double phase);

private:
    double frequency, sampleRate, phase, phaseIncrement;

    void computePhaseIncrement();
};

#endif // OSCILLATOR_H
