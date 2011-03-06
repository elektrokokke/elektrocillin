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
    class ChangeSustainPositionEvent : public RingBufferEvent
    {
    public:
        double sustainPosition;
    };

    Envelope(double sampleRate = 44100);

    LinearInterpolator * getInterpolator();
    void setInterpolator(const LinearInterpolator &interpolator);

    void setSustainPosition(double sustainPosition);
    double getSustainPosition() const;
    double getSustainPositionInSeconds() const;

    void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    virtual void processEvent(const Interpolator::ChangeControlPointEvent *event, jack_nframes_t time);
    virtual void processEvent(const Interpolator::ChangeAllControlPointsEvent *event, jack_nframes_t time);
    virtual void processEvent(const ChangeSustainPositionEvent *event, jack_nframes_t time);
private:
    double currentTime, sustainPositionInSeconds, sustainPosition, previousLevel, minimumLevel, velocity;
    Phase currentPhase;
    bool release;
    LinearInterpolator interpolator;
};

#endif // ENVELOPE_H
