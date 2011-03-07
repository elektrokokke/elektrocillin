#ifndef ENVELOPE_H
#define ENVELOPE_H

#include "midiprocessor.h"
#include "linearinterpolator.h"
#include "logarithmicinterpolator.h"

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

    Envelope(double durationInSeconds = 20, double sampleRate = 44100);

    void save(QDataStream &stream);
    void load(QDataStream &stream);

    Interpolator * getInterpolator();
    void copyInterpolator(const Envelope *envelope);

    void setSustainPosition(double sustainPosition);
    double getSustainPosition() const;
    double getSustainPositionInSeconds() const;

    double getDurationInSeconds() const;

    void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    Interpolator::ChangeAllControlPointsEvent * createIncreaseControlPointsEvent() const;
    Interpolator::ChangeAllControlPointsEvent * createDecreaseControlPointsEvent() const;

    virtual void processEvent(const Interpolator::ChangeControlPointEvent *event, jack_nframes_t time);
    virtual void processEvent(const Interpolator::ChangeAllControlPointsEvent *event, jack_nframes_t time);
    virtual void processEvent(const ChangeSustainPositionEvent *event, jack_nframes_t time);
private:
    double durationInSeconds;
    double currentTime, sustainPositionInSeconds, sustainPosition, previousLevel, minimumLevel, velocity;
    Phase currentPhase;
    bool release;
    //LinearInterpolator interpolator;
    LogarithmicInterpolator interpolator;
};

#endif // ENVELOPE_H
