#ifndef ADSRENVELOPE_H
#define ADSRENVELOPE_H

#include "notetriggered.h"

class AdsrEnvelope : public NoteTriggered
{
public:
    AdsrEnvelope(double attackTime, double decayTime, double sustainLevel, double releaseTime, double sampleRate = 44100);

    enum CurrentSegment {
        NONE,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    };

    void noteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    void noteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    void process(const double *inputs, double *outputs);

private:
    // all times are in seconds, all levels between 0 and 1:
    double attackTime, decayTime, sustainLevel, releaseTime;
    double currentSegmentTime, previousLevel, previousSegmentLevel;
    CurrentSegment currentSegment;
};

#endif // ADSRENVELOPE_H
