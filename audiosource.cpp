#include "audiosource.h"

AudioSource::AudioSource() :
    sampleRate(44100.0),
    sampleDuration(1.0 / 44100.0)
{
}

void AudioSource::setSampleRate(double sampleRate)
{
    this->sampleRate = sampleRate;
    sampleDuration = 1.0 / sampleRate;
}

double AudioSource::getSampleRate() const
{
    return sampleRate;
}

double AudioSource::getSampleDuration() const
{
    return sampleDuration;
}
