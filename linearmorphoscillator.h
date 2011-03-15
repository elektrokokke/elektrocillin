#ifndef LINEARMORPHOSCILLATOR_H
#define LINEARMORPHOSCILLATOR_H

#include "integraloscillator.h"
#include "linearinterpolator.h"

class LinearMorphOscillator : public IntegralOscillator
{
public:
    class ChangeControlPointEvent : public InterpolatorProcessor::ChangeControlPointEvent
    {
    public:
        ChangeControlPointEvent(int state_, int index_, double x_, double y_) :
            InterpolatorProcessor::ChangeControlPointEvent(index_, x_, y_),
            state(state_)
        {}
        int state;
    };

    LinearMorphOscillator(const LinearInterpolator &state1, const LinearInterpolator &state2, double sampleRate = 44100);

    const LinearInterpolator & getState(int state) const;
    void setState(int state, const LinearInterpolator &interpolator);

    // reimplemented from Oscillator, to make the morph value controllable by MIDI:
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    // reimplemented from Oscillator, to control morph by audio input:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from LinearOscillator:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
private:
    LinearInterpolator state[2];
    double morphAudio, morphMidi;

    void computeMorphedState();
};

#endif // LINEARMORPHOSCILLATOR_H
