#include "adsrenvelope.h"

AdsrEnvelope::AdsrEnvelope(double attackTime_, double decayTime_, double sustainLevel_, double releaseTime_) :
    attackTime(attackTime_),
    decayTime(decayTime_),
    sustainLevel(sustainLevel_),
    releaseTime(releaseTime_),
    preAttackTime(0.02),
    currentSegmentTime(0.0),
    previousLevel(0.0),
    previousSegmentLevel(0.0),
    currentSegment(NONE)
{
}

double AdsrEnvelope::nextSample()
{
    double level = 0.0;
    if (currentSegment == ATTACK) {
        if (currentSegmentTime >= attackTime) {
            currentSegmentTime = 0.0;
            currentSegment = DECAY;
        } else {
            // fade from previous segment level to 1:
            level = previousSegmentLevel + (1.0 - previousSegmentLevel) * currentSegmentTime / attackTime;
        }
    }
    if (currentSegment == DECAY) {
        if (currentSegmentTime >= decayTime) {
            currentSegmentTime = 0.0;
            currentSegment = SUSTAIN;
        } else {
            // fade from 1.0 to sustainLevel:
            level = 1.0 - (1.0 - sustainLevel) * currentSegmentTime / decayTime;
        }
    }
    if (currentSegment == SUSTAIN) {
        level = sustainLevel;
    }
    if (currentSegment == RELEASE) {
        if (currentSegmentTime >= releaseTime) {
            currentSegmentTime = 0.0;
            currentSegment = NONE;
        } else {
            // fade from previous segment level to 0:
            level = previousSegmentLevel - previousSegmentLevel * currentSegmentTime / releaseTime;
        }
    }
    currentSegmentTime += getSampleDuration();
    previousLevel = level;
    return level;
}

void AdsrEnvelope::noteOn()
{
    currentSegmentTime = 0.0;
    previousSegmentLevel = previousLevel;
    currentSegment = ATTACK;
}

void AdsrEnvelope::noteOff()
{
    currentSegmentTime = 0.0;
    previousSegmentLevel = previousLevel;
    currentSegment = RELEASE;
}
