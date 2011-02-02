#ifndef IIRCUTOFFCONTROLMOOGFILTER_H
#define IIRCUTOFFCONTROLMOOGFILTER_H

#include "iirmoogfilter.h"

class IIRCutoffControlMoogFilter : public IIRMoogFilter
{
public:
    IIRCutoffControlMoogFilter(double cutoffFrequencyInHertz, double resonance, double sampleRate = 44100, int zeros = 0);

    // reimplemented from IIRFilter:
    virtual void processAudio(const double *inputs, double *outputs);

    void setBaseCutoffFrequency(double baseFrequency);
    double getBaseCutoffFrequency() const;
    void setCutoffModulation(double modulation);
    double getCutoffModulation() const;
    void setModulationIntensity(double modulationIntensity);
    double getModulationIntensity() const;

private:
    double baseFrequency, modulation, modulationIntensity;
};

#endif // IIRCUTOFFCONTROLMOOGFILTER_H
