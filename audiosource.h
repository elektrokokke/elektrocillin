#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

class AudioSource
{
public:
    AudioSource();

    virtual void setSampleRate(double sampleRate);
    double getSampleRate() const;
    double getSampleDuration() const;

    virtual double nextSample() = 0;

private:
    double sampleRate, sampleDuration;
};

#endif // AUDIOSOURCE_H
