#ifndef ADSRENVELOPE_H
#define ADSRENVELOPE_H

#include "audiosource.h"

class AdsrEnvelope : public AudioSource
{
public:
    AdsrEnvelope(double attackTime, double decayTime, double sustainLevel, double releaseTime);

    enum CurrentSegment {
        NONE,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    };

    double nextSample();

    void noteOn();
    void noteOff();

private:
    // all times are in seconds, all levels between 0 and 1:
    double attackTime, decayTime, sustainLevel, releaseTime;
    double currentSegmentTime, previousLevel, previousSegmentLevel;
    CurrentSegment currentSegment;
};

#endif // ADSRENVELOPE_H
