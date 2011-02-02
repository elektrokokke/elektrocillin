#include "iircutoffcontrolmoogfilter.h"

IIRCutoffControlMoogFilter::IIRCutoffControlMoogFilter(double cutoffFrequencyInHertz, double resonance, double sampleRate, int zeros) :
    IIRMoogFilter(cutoffFrequencyInHertz, resonance, 2, sampleRate, zeros),
    baseFrequency(cutoffFrequencyInHertz),
    modulation(0),
    intensityInOctaves(1)
{
}

void IIRCutoffControlMoogFilter::processAudio(const double *inputs, double *outputs)
{
    // modify cutoff frequency from second input:
    setCutoffModulation(inputs[1]);
    IIRMoogFilter::processAudio(inputs, outputs);
}

void IIRCutoffControlMoogFilter::setCutoffFrequency(double cutoffFrequencyInHertz, double resonance)
{
    this->baseFrequency = cutoffFrequencyInHertz;
    IIRMoogFilter::setCutoffFrequency(baseFrequency * pow(1 + intensityInOctaves, modulation), resonance);
}

double IIRCutoffControlMoogFilter::getBaseCutoffFrequency() const
{
    return baseFrequency;

}

void IIRCutoffControlMoogFilter::setCutoffModulation(double modulation)
{
    this->modulation = modulation;
    setCutoffFrequency(baseFrequency, getResonance());
}

double IIRCutoffControlMoogFilter::getCutoffModulation() const
{
    return modulation;
}

void IIRCutoffControlMoogFilter::setModulationIntensity(double intensityInOctaves)
{
    this->intensityInOctaves = intensityInOctaves;
    setCutoffFrequency(baseFrequency, getResonance());
}

double IIRCutoffControlMoogFilter::getModulationIntensity() const
{
    return intensityInOctaves;
}
