#include "linearmorphoscillator.h"

LinearMorphOscillator::LinearMorphOscillator(const LinearInterpolator &state1, const LinearInterpolator &state2, double sampleRate) :
    LinearOscillator(sampleRate, QStringList("Morph modulation")),
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

bool LinearMorphOscillator::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const ChangeControlPointEvent *event_ = dynamic_cast<const ChangeControlPointEvent*>(event)) {
        InterpolatorProcessor::changeControlPoint(&state[event_->state], event_);
        return true;
    } else if (const InterpolatorProcessor::InterpolatorEvent *event_ = dynamic_cast<const InterpolatorProcessor::InterpolatorEvent*>(event)) {
        InterpolatorProcessor::processInterpolatorEvent(&state[0], event_);
        InterpolatorProcessor::processInterpolatorEvent(&state[1], event_);
        return true;
    } else {
        return LinearOscillator::processEvent(event, time);
    }
}

void LinearMorphOscillator::computeMorphedState()
{
    Q_ASSERT_X(state[0].getX().size() == state[1].getY().size(), "void LinearMorphOscillator::computeMorphedState(double morph)", "Both morph states have to have the same number of control points");
    QVector<double> xx, yy;
    xx.resize(state[0].getX().size());
    yy.resize(state[0].getX().size());
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
        xx[i] = state[0].getX()[i] * weight1 + state[1].getX()[i] * weight2;
        yy[i] = state[0].getY()[i] * weight1 + state[1].getY()[i] * weight2;
    }
    morphedState.changeControlPoints(xx, yy);
    changeControlPoints(xx, yy);
}
