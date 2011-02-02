#ifndef IIRBUTTERWORTHFILTER_H
#define IIRBUTTERWORTHFILTER_H

#include "iirfilter.h"

class IIRButterworthFilter : public IIRFilter
{
public:
    enum Type {
        LOW_PASS,
        HIGH_PASS
    };

    IIRButterworthFilter(double cutoffFrequencyInHertz, double sampleRate, Type type = LOW_PASS);

    virtual void setCutoffFrequency(double cutoffFrequencyInHertz, Type type);

    void setCutoffFrequency(double cutoffFrequencyInHertz);
    void setType(Type type);

    double getCutoffFrequency() const;
    Type getType() const;

private:
    double cutoffFrequency;
    Type type;
};

#endif // IIRBUTTERWORTHFILTER_H
