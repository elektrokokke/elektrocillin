#ifndef LINEARMORPHOSCILLATOR_H
#define LINEARMORPHOSCILLATOR_H

#include "linearoscillator.h"

class LinearMorphOscillator : public LinearOscillator
{
public:
    LinearMorphOscillator(const LinearOscillator &state1, const LinearOscillator &state2, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);
    LinearMorphOscillator(const LinearInterpolator &state1, const LinearInterpolator &state2, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);

    // Reimplemented from Oscillator, to evaluate morph input:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

private:
    LinearInterpolator state1, state2, morphedState;

    void computeMorphedState(double morph);
};

#endif // LINEARMORPHOSCILLATOR_H
