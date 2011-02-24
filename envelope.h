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

    Envelope(double sampleRate = 44100);

    const LinearInterpolator & getInterpolator(Phase phase) const;

    void setSustainLevel(double sustainLevel);
    double getSustainLevel() const;

    void setDuration(Phase phase, double duration);
    double getDuration(Phase phase) const;
    void setDuration(double duration);
    double getDuration() const;

    void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

private:
    double currentPhaseTime, sustainLevel, previousLevel, minimumLevel, velocity;
    Phase currentPhase;
    bool release;
    LinearInterpolator interpolator[2];
};

#endif // ENVELOPE_H
