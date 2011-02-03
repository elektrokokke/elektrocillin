#ifndef IIRMOOGFILTER_H
#define IIRMOOGFILTER_H

#include "iirfilter.h"

class IIRMoogFilter : public IIRFilter
{
public:
    struct Parameters {
        double frequency;
        double frequencyFactor;
        double frequencyModulation;
        double frequencyModulationIntensity;
        double resonance;
    };

    IIRMoogFilter(double sampleRate = 44100, int zeros = 0);

    // reimplemented from IIRFilter:
    virtual void processAudio(const double *inputs, double *outputs);

    void setSampleRate(double sampleRate);

    void setParameters(const Parameters parameters);
    const Parameters & getParameters() const;

    void computeCoefficients();
private:
    Parameters parameters;
};

#endif // IIRMOOGFILTER_H
