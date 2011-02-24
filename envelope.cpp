#include "envelope.h"

Envelope::Envelope(double sampleRate) :
        MidiProcessor(QStringList(), QStringList("Envelope out"), sampleRate),
        currentPhaseTime(0),
        sustainLevel(0.5),
        previousLevel(0),
        minimumLevel(0),
        velocity(0)
{
    // initialize the interpolators to represent a simple ASR envelope:
    QVector<double> xx[2], yy[2];
    xx[ATTACK].append(0);
    yy[ATTACK].append(0);
    xx[ATTACK].append(0.1);
    yy[ATTACK].append(sustainLevel);
    xx[RELEASE].append(0);
    yy[RELEASE].append(sustainLevel);
    xx[RELEASE].append(1);
    yy[RELEASE].append(0);
    for (int i = 0; i < 2; i++) {
        interpolator[i] = LinearInterpolator(xx[i], yy[i]);
    }
}

const LinearInterpolator & Envelope::getInterpolator(Phase phase) const
{
    Q_ASSERT((phase == ATTACK) || (phase == RELEASE));
    return interpolator[phase];
}

void Envelope::setSustainLevel(double sustainLevel)
{
    this->sustainLevel = sustainLevel;
    interpolator[ATTACK].getY().back() = sustainLevel;
    interpolator[RELEASE].getY().first() = sustainLevel;
}

double Envelope::getSustainLevel() const
{
    return sustainLevel;
}

void Envelope::setDuration(Phase phase, double duration)
{
    Q_ASSERT((phase == ATTACK) || (phase == RELEASE));
    // stretch the attack interpolator to have the given duration:
    double stretchFactor = duration / getDuration(phase);
    for (int i = 0; i < interpolator[phase].getX().size(); i++) {
        interpolator[phase].getX()[i] *= stretchFactor;
    }
}

double Envelope::getDuration(Phase phase) const
{
    Q_ASSERT((phase == ATTACK) || (phase == RELEASE));
    return interpolator[phase].getX().back();
}

void Envelope::setDuration(double duration)
{
    double attackDuration = getDuration(ATTACK);
    double releaseDuration = getDuration(RELEASE);
    double oldDuration = attackDuration + releaseDuration;
    setDuration(ATTACK, attackDuration * duration / oldDuration);
    setDuration(RELEASE, releaseDuration * duration / oldDuration);
}

double Envelope::getDuration() const
{
    return getDuration(ATTACK) + getDuration(RELEASE);
}

void Envelope::processNoteOn(unsigned char, unsigned char, unsigned char velocity, jack_nframes_t)
{
    double newVelocity = velocity / 127.0;
    currentPhaseTime = 0.0;
    currentPhase = ATTACK;
    minimumLevel = previousLevel * this->velocity / newVelocity;
    this->velocity = newVelocity;
    release = false;
}

void Envelope::processNoteOff(unsigned char, unsigned char, unsigned char, jack_nframes_t)
{
    release = true;
}

void Envelope::processAudio(const double *, double *outputs, jack_nframes_t)
{
    double level = 0.0;
    if (currentPhase == ATTACK) {
        if (currentPhaseTime >= interpolator[ATTACK].getX().back()) {
            currentPhaseTime = 0.0;
            currentPhase = SUSTAIN;
        } else {
            level = interpolator[ATTACK].evaluate(currentPhaseTime);
        }
    }
    if (currentPhase == SUSTAIN) {
        if (release) {
            currentPhaseTime = 0.0;
            currentPhase = RELEASE;
        } else {
            level = sustainLevel;
        }
    }
    if (currentPhase == RELEASE) {
        if (currentPhaseTime >= interpolator[RELEASE].getX().back()) {
            currentPhaseTime = 0.0;
            currentPhase = NONE;
        } else {
            level = interpolator[RELEASE].evaluate(currentPhaseTime);
        }
    }
    currentPhaseTime += getSampleDuration();
    previousLevel = level;
    outputs[0] = level * velocity;
}
