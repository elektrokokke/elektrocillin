#ifndef IIRBUTTERWORTHFILTER_H
#define IIRBUTTERWORTHFILTER_H

#include "iirfilter.h"

class IIRButterworthFilter : public IIRFilter
{
public:
    IIRButterworthFilter(double cutoffFrequencyInHertz, double sampleRate);

    void setCutoffFrequency(double cutoffFrequencyInHertz);

private:
};

#endif // IIRBUTTERWORTHFILTER_H
