#ifndef IIRMOOGFILTER_H
#define IIRMOOGFILTER_H

#include "iirfilter.h"

class IIRMoogFilter : public IIRFilter
{
public:
    IIRMoogFilter(double cutoffFrequencyInHertz, double resonance, double sampleRate, int zeros = 0);

    void setCutoffFrequency(double cutoffFrequencyInHertz);
    void setCutoffFrequency(double cutoffFrequencyInHertz, double resonance);
    void setResonance(double resonance);

    double getCutoffFrequency() const;
    double getResonance() const;

private:
    double cutoffFrequencyInHertz, resonance;
};

#endif // IIRMOOGFILTER_H
