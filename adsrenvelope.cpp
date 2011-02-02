#include "adsrenvelope.h"

AdsrEnvelope::AdsrEnvelope(double attackTime_, double decayTime_, double sustainLevel_, double releaseTime_, double sampleRate) :
    MidiProcessor(0, 1, sampleRate),
    attackTime(attackTime_),
    decayTime(decayTime_),
    sustainLevel(sustainLevel_),
    releaseTime(releaseTime_),
    currentSegmentTime(0.0),
    previousLevel(0.0),
    previousSegmentLevel(0.0),
    currentSegment(NONE)
{
}

void AdsrEnvelope::processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity)
{
    currentSegmentTime = 0.0;
    currentSegment = ATTACK;
}

void AdsrEnvelope::processNoteOff(unsigned char, unsigned char, unsigned char velocity)
{
    currentSegmentTime = 0.0;
    previousSegmentLevel = previousLevel;
    currentSegment = RELEASE;
}

void AdsrEnvelope::processAudio(const double *, double *outputs)
{
    double level = 0.0;
    if (currentSegment == ATTACK) {
        if (currentSegmentTime >= attackTime) {
            currentSegmentTime = 0.0;
            currentSegment = DECAY;
        } else {
            // fade from 0 to 1:
            level = currentSegmentTime / attackTime;
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
    outputs[0] = level;
}
