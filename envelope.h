#ifndef ENVELOPE_H
#define ENVELOPE_H

#include "midiprocessor.h"
#include "linearinterpolator.h"

class Envelope : public MidiProcessor
{
public:
    enum Phase {
        ATTACK = 0,
        RELEASE = 1,
        NONE,
        SUSTAIN
    };
    class ChangeDurationEvent : public RingBufferEvent
    {
    public:
        double duration;
    };
    class ChangeSustainPositionEvent : public RingBufferEvent
    {
    public:
        double sustainPosition;
    };

    Envelope(double sampleRate = 44100);

    LinearInterpolator * getInterpolator();

    void setSustainPosition(double sustainPosition);
    double getSustainPosition() const;

    void setDuration(double duration);
    double getDuration() const;

    void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    virtual void processEvent(const Interpolator::ChangeControlPointEvent *event, jack_nframes_t time);
    virtual void processEvent(const Interpolator::ChangeAllControlPointsEvent *event, jack_nframes_t time);
    virtual void processEvent(const ChangeDurationEvent *event, jack_nframes_t time);
    virtual void processEvent(const ChangeSustainPositionEvent *event, jack_nframes_t time);
private:
    double currentPhaseTime, sustainPosition, previousLevel, minimumLevel, velocity;
    Phase currentPhase;
    bool release;
    LinearInterpolator interpolator;
};

#endif // ENVELOPE_H
