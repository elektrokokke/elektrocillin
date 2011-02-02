#ifndef IIRMOOGFILTER_H
#define IIRMOOGFILTER_H

#include "iirfilter.h"

class IIRMoogFilter : public IIRFilter
{
public:
    IIRMoogFilter(double cutoffFrequencyInHertz, double resonance, int nrOfInputs = 1, double sampleRate = 44100, int zeros = 0);

    void setSampleRate(double sampleRate);

    virtual void setCutoffFrequency(double cutoffFrequencyInHertz, double resonance);
    double getCutoffFrequency() const;
    double getResonance() const;

private:
    double cutoffFrequencyInHertz, resonance;
};

#endif // IIRMOOGFILTER_H
