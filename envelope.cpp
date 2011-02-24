#include "envelope.h"
#include <QtGlobal>

Envelope::Envelope(double sampleRate) :
        MidiProcessor(QStringList(), QStringList("Envelope out"), sampleRate),
        currentPhaseTime(0),
        sustainPosition(0.5),
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
    xx.append(1);
    yy.append(0);
    interpolator = LinearInterpolator(xx, yy);
}

LinearInterpolator * Envelope::getInterpolator()
{
    return &interpolator;
}

void Envelope::setSustainPosition(double sustainPosition)
{
    this->sustainPosition = sustainPosition;
}

double Envelope::getSustainPosition() const
{
    return sustainPosition;
}

void Envelope::setDuration(double duration)
{
    if (duration < interpolator.getX()[interpolator.getX().size() - 2]) {
        // stretch the interpolator to have the given duration:
        double stretchFactor = duration / getDuration();
        for (int i = 0; i < interpolator.getX().size(); i++) {
            interpolator.getX()[i] *= stretchFactor;
        }
    } else {
        interpolator.getX().back() = duration;
    }
    if (sustainPosition > duration) {
        sustainPosition = duration;
    }
}

double Envelope::getDuration() const
{
    return interpolator.getX().back();
}

void Envelope::processNoteOn(unsigned char, unsigned char, unsigned char velocity, jack_nframes_t)
{
    double newVelocity = velocity / 127.0;
    currentPhaseTime = 0.0;
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
        if (currentPhaseTime >= sustainPosition) {
            currentPhaseTime = 0.0;
            currentPhase = SUSTAIN;
        } else {
            level = interpolator.evaluate(currentPhaseTime);
            if (qAbs(level) < minimumLevel) {
                level = (level < 0.0 ? -minimumLevel : minimumLevel);
            } else {
                minimumLevel = 0;
            }
        }
    }
    if (currentPhase == SUSTAIN) {
        if (release) {
            currentPhaseTime = 0.0;
            currentPhase = RELEASE;
        } else {
            level = interpolator.evaluate(sustainPosition);
        }
    }
    if (currentPhase == RELEASE) {
        if (currentPhaseTime + sustainPosition >= getDuration()) {
            currentPhaseTime = 0.0;
            currentPhase = NONE;
        } else {
            level = interpolator.evaluate(currentPhaseTime + sustainPosition);
        }
    }
    currentPhaseTime += getSampleDuration();
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

void Envelope::processEvent(const ChangeDurationEvent *event, jack_nframes_t)
{
    setDuration(event->duration);
}

void Envelope::processEvent(const ChangeSustainPositionEvent *event, jack_nframes_t)
{
    setSustainPosition(event->sustainPosition);
}
