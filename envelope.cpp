#include "envelope.h"
#include <QtGlobal>
#include <cmath>

Envelope::Envelope(double durationInSeconds_, double sampleRate) :
    MidiProcessor(QStringList(), QStringList("Envelope out"), sampleRate),
    durationInSeconds(durationInSeconds_),
    currentTime(0),
    // sustain position is at 0.5 seconds:
    sustainPositionInSeconds(0.5),
    sustainPosition(log(sustainPositionInSeconds + 1)),
    previousLevel(0),
    minimumLevel(0),
    velocity(0),
    interpolator(0.01)
{
    // initialize the interpolators to represent a simple ASR envelope:
    Interpolator::ChangeAllControlPointsEvent initEvent;
    initEvent.xx.append(0);
    initEvent.yy.append(0);
    initEvent.xx.append(sustainPosition);
    initEvent.yy.append(1);
    initEvent.xx.append(log(durationInSeconds + 1));
    initEvent.yy.append(0);
    interpolator.processEvent(&initEvent);
}

void Envelope::save(QDataStream &stream)
{
    interpolator.save(stream);
    stream << sustainPosition;
}

void Envelope::load(QDataStream &stream)
{
    interpolator.load(stream);
    stream >> sustainPosition;
}

Interpolator * Envelope::getInterpolator()
{
    return &interpolator;
}

void Envelope::copyInterpolator(const Envelope *envelope)
{
    interpolator = envelope->interpolator;
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

double Envelope::getDurationInSeconds() const
{
    return durationInSeconds;
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
        if (x >= qMin(sustainPosition, interpolator.getX().last())) {
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
            level = interpolator.evaluate(qMin(sustainPosition, interpolator.getX().last()));
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

Interpolator::ChangeAllControlPointsEvent * Envelope::createIncreaseControlPointsEvent() const
{
    Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent();
    event->xx = interpolator.getX();
    event->yy = interpolator.getY();
    int size = event->xx.size() + 1;
    double stretchFactor = (double)(event->xx.size() - 1) / (double)(size - 1);
    event->xx.append(event->xx.back());
    event->yy.append(event->yy.back());
    for (int i = size - 1; i >= 0; i--) {
        if (i < size - 1) {
            event->xx[i] = event->xx[i] * stretchFactor;
        }
    }
    return event;
}

Interpolator::ChangeAllControlPointsEvent * Envelope::createDecreaseControlPointsEvent() const
{
    Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent();
    event->xx = interpolator.getX();
    event->yy = interpolator.getY();
    int size = event->xx.size() - 1;
    double stretchFactor = event->xx.back() / event->xx[size - 1];
    event->xx.resize(size);
    event->yy.resize(size);
    for (int i = size - 1; i >= 0; i--) {
        event->xx[i] = event->xx[i] * stretchFactor;
    }
    event->yy.back() = 0;
    return event;
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
