#ifndef ENVELOPE_H
#define ENVELOPE_H

#include "audioprocessor.h"
#include "midiprocessor.h"
#include "eventprocessor.h"
#include "interpolatorprocessor.h"
#include "linearinterpolator.h"
#include "logarithmicinterpolator.h"

class Envelope : public AudioProcessor, public MidiProcessor, public EventProcessor, public InterpolatorProcessor
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
        ChangeSustainPositionEvent(int sustainIndex_) :
            sustainIndex(sustainIndex_)
        {}
        int sustainIndex;
    };

    Envelope(double durationInSeconds = 20, double sampleRate = 44100);

    void save(QDataStream &stream);
    void load(QDataStream &stream);

    Interpolator * getInterpolator();
    void copyInterpolatorFrom(const Envelope *envelope);

    int getSustainIndex() const;
    void setSustainIndex(int sustainIndex);

    double getDurationInSeconds() const;

    // reimplemented from MidiProcessor:
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from EventProcessor:
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
