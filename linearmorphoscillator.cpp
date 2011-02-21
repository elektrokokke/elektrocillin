#include "linearmorphoscillator.h"

LinearMorphOscillator::LinearMorphOscillator(const LinearOscillator &state1, const LinearOscillator &state2, double frequencyModulationIntensity, double sampleRate) :
    LinearOscillator(frequencyModulationIntensity, sampleRate, QStringList("Morph modulation"))
{
    state[0] = state1.getLinearInterpolator();
    state[1] = state2.getLinearInterpolator();
}

LinearMorphOscillator::LinearMorphOscillator(const LinearInterpolator &state1, const LinearInterpolator &state2, double frequencyModulationIntensity, double sampleRate) :
    LinearOscillator(frequencyModulationIntensity, sampleRate, QStringList("Morph modulation"))
{
    state[0] = state1;
    state[1] = state2;
}

const LinearInterpolator & LinearMorphOscillator::getState(int stateIndex) const
{
    Q_ASSERT((stateIndex >= 0) && (stateIndex < 2));
    return state[stateIndex];
}

void LinearMorphOscillator::setState(int stateIndex, const LinearInterpolator &interpolator)
{
    Q_ASSERT((stateIndex >= 0) && (stateIndex < 2));
    state[stateIndex] = interpolator;
}

void LinearMorphOscillator::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    // compute the morphed state:
    computeMorphedState(inputs[1]);
    // call the base implementation:
    LinearOscillator::processAudio(inputs, outputs, time);
}

void LinearMorphOscillator::processEvent(const LinearMorphOscillatorParameters &event, jack_nframes_t)
{
    Q_ASSERT((event.state >= 0) && (event.state < 2));
    // set the interpolator's nr of control points:
    state[event.state].getX().resize(event.controlPoints);
    state[event.state].getY().resize(event.controlPoints);
    // set the interpolator control point at "index" accordingly:
    state[event.state].getX()[event.index] = event.x;
    state[event.state].getY()[event.index] = event.y;
}

void LinearMorphOscillator::computeMorphedState(double morph)
{
    Q_ASSERT_X(state[0].getX().size() == state[1].getY().size(), "void LinearMorphOscillator::computeMorphedState(double morph)", "Both morph states have to have the same number of control points");
    morphedState.getX().resize(state[0].getX().size());
    morphedState.getY().resize(state[0].getX().size());
    // let the morphed state be a weighted average of both states (-1 = state1, 1 = state2, 0 = both states weighted equally):
    double weight1 = 0.5 - morph * 0.5;
    double weight2 = 0.5 + morph * 0.5;
    for (int i = 0; i < morphedState.getX().size(); i++) {
        morphedState.getX()[i] = state[0].getX()[i] * weight1 + state[1].getX()[i] * weight2;
        morphedState.getY()[i] = state[0].getY()[i] * weight1 + state[1].getY()[i] * weight2;
    }
    setLinearInterpolator(morphedState);
}
