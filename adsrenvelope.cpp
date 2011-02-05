#include "adsrenvelope.h"

AdsrEnvelope::AdsrEnvelope(double attackTime_, double decayTime_, double sustainLevel_, double releaseTime_, double sampleRate) :
    MidiProcessor(QStringList(), QStringList("envelope_out"), sampleRate),
    attackTime(attackTime_),
    decayTime(decayTime_),
    sustainLevel(sustainLevel_),
    releaseTime(releaseTime_),
    currentSegmentTime(0),
    previousLevel(0),
    attackStartLevel(0),
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
    attackStartLevel = previousLevel;
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
            // fade from attackStartLevel to 1:
            level = attackStartLevel + (1.0 - attackStartLevel) * currentSegmentTime / attackTime;
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
            currentSegmentTime = 0.0;
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
    previousLevel = level;
    outputs[0] = level * velocity;
}
