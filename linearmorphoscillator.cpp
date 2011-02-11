#include "linearmorphoscillator.h"

LinearMorphOscillator::LinearMorphOscillator(const LinearOscillator &state1_, const LinearOscillator &state2_, double frequencyModulationIntensity, double sampleRate) :
    LinearOscillator(frequencyModulationIntensity, sampleRate, QStringList("Morph modulation")),
    state1(state1_.getLinearInterpolator()),
    state2(state2_.getLinearInterpolator()),
    morphedState(state1.getX(), state1.getY())
{
    computeMorphedState(0);
}

LinearMorphOscillator::LinearMorphOscillator(const LinearInterpolator &state1_, const LinearInterpolator &state2_, double frequencyModulationIntensity, double sampleRate) :
    LinearOscillator(frequencyModulationIntensity, sampleRate, QStringList("Morph modulation")),
    state1(state1_),
    state2(state2_),
    morphedState(state1.getX(), state1.getY())
{
    computeMorphedState(0);
}

void LinearMorphOscillator::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    // compute the morphed state:
    computeMorphedState(inputs[2]);
    // call the base implementation:
    LinearOscillator::processAudio(inputs, outputs, time);
}

void LinearMorphOscillator::computeMorphedState(double morph)
{
    Q_ASSERT_X(state1.getX().size() == state2.getY().size(), "void LinearMorphOscillator::computeMorphedState(double morph)", "Both morph states have to have the same number of control points");
    // let the morphed state be a weighted average of both states (-1 = state1, 1 = state2, 0 = both states weighted equally):
    double weight1 = 0.5 - morph * 0.5;
    double weight2 = 0.5 + morph * 0.5;
    for (int i = 0; i < morphedState.getX().size(); i++) {
        morphedState.getX()[i] = state1.getX()[i] * weight1 + state2.getX()[i] * weight2;
        morphedState.getY()[i] = state1.getY()[i] * weight1 + state2.getY()[i] * weight2;
    }
    setLinearInterpolator(morphedState);
}
