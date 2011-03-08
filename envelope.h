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
        int sustainIndex;
    };

    Envelope(double durationInSeconds = 20, double sampleRate = 44100);

    void save(QDataStream &stream);
    void load(QDataStream &stream);

    Interpolator * getInterpolator();
    void copyInterpolator(const Envelope *envelope);

    void setSustainIndex(int sustainIndex);

    double getDurationInSeconds() const;

    void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
private:
    double durationInSeconds;
    double currentTime, previousLevel, minimumLevel, velocity;
    int sustainIndex;
    Phase currentPhase;
    bool release;
    //LinearInterpolator interpolator;
    LogarithmicInterpolator interpolator;
};

#endif // ENVELOPE_H
