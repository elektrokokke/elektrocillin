#ifndef PULSEOSCILLATOR2_H
#define PULSEOSCILLATOR2_H

#include "linearinterpolator.h"
#include "oscillator.h"

class PulseOscillator2 : public Oscillator
{
public:
    PulseOscillator2(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);

    void setPulseWidth(double pulseWidth);
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

protected:
    double valueAtPhase(double phase);

private:
    double previousPhase, previousIntegralValue;
    LinearInterpolator pulseIntegral;
};

#endif // PULSEOSCILLATOR2_H
