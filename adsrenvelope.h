#ifndef ADSRENVELOPE_H
#define ADSRENVELOPE_H

#include "midiprocessor.h"

class AdsrEnvelope : public MidiProcessor
{
public:
    AdsrEnvelope(double attackTime, double decayTime, double sustainLevel, double releaseTime, double sampleRate = 44100);

    void setAttackTime(double value);
    void setDecayTime(double value);
    void setSustainLevel(double value);
    void setReleaseTime(double value);

    enum CurrentSegment {
        NONE,
        PREATTACK,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    };

    void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

private:
    // all times are in seconds, all levels between 0 and 1:
    double preAttackTime, attackTime, decayTime, sustainLevel, releaseTime;
    double currentSegmentTime;
    double previousLevel, attackStartLevel;
    double velocity;
    CurrentSegment currentSegment;
    bool release;
};

#endif // ADSRENVELOPE_H
