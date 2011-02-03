#include "adsrenvelope.h"

AdsrEnvelope::AdsrEnvelope(double attackTime_, double decayTime_, double sustainLevel_, double releaseTime_, double sampleRate) :
    MidiProcessor(0, 1, sampleRate),
    attackTime(attackTime_),
    decayTime(decayTime_),
    sustainLevel(sustainLevel_),
    releaseTime(releaseTime_),
    currentSegmentTime(0),
    velocity(1),
    currentSegment(NONE),
    release(false)
{
}

void AdsrEnvelope::processNoteOn(unsigned char, unsigned char, unsigned char velocity, jack_nframes_t)
{
    this->velocity = velocity / 127.0;
    currentSegmentTime = 0.0;
    currentSegment = ATTACK;
    release = false;
}

void AdsrEnvelope::processNoteOff(unsigned char, unsigned char, unsigned char, jack_nframes_t)
{
    release = true;
}

void AdsrEnvelope::processAudio(const double *, double *outputs, jack_nframes_t)
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
        if (release) {
            currentSegment = RELEASE;
        } else {
            level = sustainLevel;
        }
    }
    if (currentSegment == RELEASE) {
        if (currentSegmentTime >= releaseTime) {
            currentSegmentTime = 0.0;
            currentSegment = NONE;
        } else {
            // fade from sustain level to 0:
            level = sustainLevel - sustainLevel * currentSegmentTime / releaseTime;
        }
    }
    currentSegmentTime += getSampleDuration();
    outputs[0] = level * velocity;
}
