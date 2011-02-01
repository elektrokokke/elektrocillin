#ifndef IIRMOOGFILTER_H
#define IIRMOOGFILTER_H

#include "iirfilter.h"

class IIRMoogFilter : public IIRFilter
{
public:
    IIRMoogFilter(double cutoffFrequencyInHertz, double resonance, double sampleRate, int zeros = 0);

    virtual void setSampleRate(double sampleRate);
    void setCutoffFrequency(double cutoffFrequencyInHertz, double resonance);

    void setCutoffFrequency(double cutoffFrequencyInHertz);
    void setResonance(double resonance);

    double getCutoffFrequency() const;
    double getResonance() const;

private:
    double cutoffFrequencyInHertz, resonance;
};

#endif // IIRMOOGFILTER_H
