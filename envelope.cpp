#include "envelope.h"
#include <QtGlobal>
#include <cmath>

Envelope::Envelope(double sampleRate) :
    MidiProcessor(QStringList(), QStringList("Envelope out"), sampleRate),
    currentTime(0),
    // sustain position is at 0.5 seconds:
    sustainPositionInSeconds(0.5),
    sustainPosition(log(sustainPositionInSeconds + 1)),
    previousLevel(0),
    minimumLevel(0),
    velocity(0)
{
    // initialize the interpolators to represent a simple ASR envelope:
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(0);
    xx.append(sustainPosition);
    yy.append(1);
    // duration is 20 seconds (without sustain phase):
    xx.append(log(20 + 1));
    yy.append(0);
    interpolator = LinearInterpolator(xx, yy);
}

LinearInterpolator * Envelope::getInterpolator()
{
    return &interpolator;
}

void Envelope::setInterpolator(const LinearInterpolator &interpolator)
{
    this->interpolator = interpolator;
}

void Envelope::setSustainPosition(double sustainPosition)
{
    this->sustainPosition = sustainPosition;
    sustainPositionInSeconds = exp(sustainPosition) - 1;
}

double Envelope::getSustainPosition() const
{
    return sustainPosition;
}

double Envelope::getSustainPositionInSeconds() const
{
    return sustainPositionInSeconds;
}

void Envelope::processNoteOn(unsigned char, unsigned char, unsigned char velocity, jack_nframes_t)
{
    double newVelocity = velocity / 127.0;
    currentTime = 0.0;
    currentPhase = ATTACK;
    minimumLevel = qAbs(previousLevel) * this->velocity / newVelocity;
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
        double x = log(currentTime + 1);
        if (x >= sustainPosition) {
            currentPhase = SUSTAIN;
        } else {
            level = interpolator.evaluate(x);
            if (qAbs(level) < minimumLevel) {
                level = (level < 0.0 ? -minimumLevel : minimumLevel);
            } else {
                minimumLevel = 0;
            }
        }
    }
    if (currentPhase == SUSTAIN) {
        if (release) {
            currentPhase = RELEASE;
            currentTime = sustainPositionInSeconds;
        } else {
            level = interpolator.evaluate(sustainPosition);
        }
    }
    if (currentPhase == RELEASE) {
        double x = log(currentTime + 1);
        if (x >= interpolator.getX().last()) {
            currentPhase = NONE;
        } else {
            level = interpolator.evaluate(x);
        }
    }
    currentTime += getSampleDuration();
    previousLevel = level;
    outputs[0] = level * velocity;
}

void Envelope::processEvent(const Interpolator::ChangeControlPointEvent *event, jack_nframes_t)
{
    interpolator.processEvent(event);
}

void Envelope::processEvent(const Interpolator::ChangeAllControlPointsEvent *event, jack_nframes_t)
{
    interpolator.processEvent(event);
}

void Envelope::processEvent(const ChangeSustainPositionEvent *event, jack_nframes_t)
{
    setSustainPosition(event->sustainPosition);
}
