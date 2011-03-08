#ifndef LINEARMORPHOSCILLATOR_H
#define LINEARMORPHOSCILLATOR_H

#include "linearoscillator.h"


class LinearMorphOscillator : public LinearOscillator
{
public:
    class ChangeControlPointEvent : public Interpolator::ChangeControlPointEvent
    {
    public:
        ChangeControlPointEvent(int state_, int index_, double x_, double y_) :
            Interpolator::ChangeControlPointEvent(index_, x_, y_),
            state(state_)
        {}
        int state;
    };
    class AddControlPointsEvent : public Interpolator::AddControlPointsEvent
    {
    public:
        AddControlPointsEvent(int state_, bool scaleX_, bool scaleY_, bool addAtStart_, bool addAtEnd_) :
            Interpolator::AddControlPointsEvent(scaleX_, scaleY_, addAtStart_, addAtEnd_),
            state(state_)
        {}
        int state;
    };
    class DeleteControlPointsEvent : public Interpolator::DeleteControlPointsEvent
    {
    public:
        DeleteControlPointsEvent(int state_, bool scaleX_, bool scaleY_, bool deleteAtStart_, bool deleteAtEnd_) :
            Interpolator::DeleteControlPointsEvent(scaleX_, scaleY_, deleteAtStart_, deleteAtEnd_),
            state(state_)
        {}
        int state;
    };

    LinearMorphOscillator(const LinearOscillator &state1, const LinearOscillator &state2, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);
    LinearMorphOscillator(const LinearInterpolator &state1, const LinearInterpolator &state2, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);

    const LinearInterpolator & getState(int state) const;
    void setState(int state, const LinearInterpolator &interpolator);

    // Reimplemented from Oscillator, to make the morph value controllable by MIDI:
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    // Reimplemented from Oscillator, to control morph by audio input:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    // Reimplemented from LinearOscillator:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
private:
    LinearInterpolator state[2], morphedState;
    double morphAudio, morphMidi;

    void computeMorphedState();
};

#endif // LINEARMORPHOSCILLATOR_H
