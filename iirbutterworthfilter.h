#ifndef IIRBUTTERWORTHFILTER_H
#define IIRBUTTERWORTHFILTER_H

#include "iirfilter.h"

class IirButterworthFilter : public IirFilter
{
public:
    enum Type {
        LOW_PASS,
        HIGH_PASS
    };

    IirButterworthFilter(double cutoffFrequencyInHertz, Type type = LOW_PASS, double sampleRate = 44100);

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
