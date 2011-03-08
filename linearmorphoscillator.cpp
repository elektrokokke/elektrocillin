#include "linearmorphoscillator.h"

LinearMorphOscillator::LinearMorphOscillator(const LinearOscillator &state1, const LinearOscillator &state2, double frequencyModulationIntensity, double sampleRate) :
    LinearOscillator(frequencyModulationIntensity, sampleRate, QStringList("Morph modulation")),
    morphAudio(0),
    morphMidi(0)
{
    state[0] = state1.getLinearInterpolator();
    state[1] = state2.getLinearInterpolator();
}

LinearMorphOscillator::LinearMorphOscillator(const LinearInterpolator &state1, const LinearInterpolator &state2, double frequencyModulationIntensity, double sampleRate) :
    LinearOscillator(frequencyModulationIntensity, sampleRate, QStringList("Morph modulation")),
    morphAudio(0),
    morphMidi(0)
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

void LinearMorphOscillator::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    if (controller == 4) {
        morphMidi = (double)value / 127.0 * 2.0 - 1.0;
    } else {
        LinearOscillator::processController(channel, controller, value, time);
    }
}

void LinearMorphOscillator::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    // compute the morphed state:
    morphAudio = inputs[1];
    computeMorphedState();
    // call the base implementation:
    LinearOscillator::processAudio(inputs, outputs, time);
}

void LinearMorphOscillator::processEvent(const ChangeControlPointEvent *event, jack_nframes_t)
{
    Q_ASSERT((event->state >= 0) && (event->state < 2));
    state[event->state].processEvent(event);
}

void LinearMorphOscillator::processEvent(const ChangeAllControlPointsEvent *event, jack_nframes_t)
{
    Q_ASSERT((event->state >= 0) && (event->state < 2));
    state[event->state].processEvent(event);
}

void LinearMorphOscillator::computeMorphedState()
{
    Q_ASSERT_X(state[0].getX().size() == state[1].getY().size(), "void LinearMorphOscillator::computeMorphedState(double morph)", "Both morph states have to have the same number of control points");
    Interpolator::ChangeAllControlPointsEvent event(morphedState);
    event.xx.resize(state[0].getX().size());
    event.yy.resize(state[0].getX().size());
    // make sure that morph lies in [-1,1] by reflecting at the top and bottom:
    double morph = morphAudio + morphMidi;
    if (morph < -1.0) {
        morph = -2.0 - morph;
    } else if (morph > 1.0) {
        morph = 2.0 - morph;
    }
    // let the morphed state be a weighted average of both states (-1 = state1, 1 = state2, 0 = both states weighted equally):
    double weight1 = 0.5 - morph * 0.5;
    double weight2 = 0.5 + morph * 0.5;
    for (int i = 0; i < morphedState.getX().size(); i++) {
        event.xx[i] = state[0].getX()[i] * weight1 + state[1].getX()[i] * weight2;
        event.yy[i] = state[0].getY()[i] * weight1 + state[1].getY()[i] * weight2;
    }
    morphedState.processEvent(&event);
    LinearOscillator::processEvent(&event, 0);
}
