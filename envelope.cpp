#include "envelope.h"
#include <QtGlobal>
#include <cmath>

Envelope::Envelope(double durationInSeconds_, double sampleRate) :
    AudioProcessor(QStringList(), QStringList("Envelope out"), sampleRate),
    durationInSeconds(durationInSeconds_),
    currentTime(0),
    previousLevel(0),
    minimumLevel(0),
    velocity(0),
    sustainIndex(1),
    interpolator(0.01)
{
    // initialize the interpolators to represent a simple ASR envelope:
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(0);
    xx.append(log(0.5 + 1));
    yy.append(1);
    xx.append(log(durationInSeconds + 1));
    yy.append(0);
    interpolator.changeControlPoints(xx, yy);
    interpolator.setControlPointName(sustainIndex, "Sustain");
    interpolator.setEndPointConstraints(false, true);
}

void Envelope::save(QDataStream &stream)
{
    interpolator.save(stream);
    stream << sustainIndex;
}

void Envelope::load(QDataStream &stream)
{
    interpolator.load(stream);
    stream >> sustainIndex;
    interpolator.setControlPointName(sustainIndex, "Sustain");
}

Interpolator * Envelope::getInterpolator()
{
    return &interpolator;
}

void Envelope::copyInterpolatorFrom(const Envelope *envelope)
{
    interpolator = envelope->interpolator;
}

void Envelope::setSustainIndex(int sustainIndex)
{
    if ((sustainIndex > 0) && (sustainIndex < interpolator.getX().size()) && (sustainIndex != this->sustainIndex)) {
        interpolator.setControlPointName(this->sustainIndex, QString());
        this->sustainIndex = sustainIndex;
        interpolator.setControlPointName(this->sustainIndex, "Sustain");
    }
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
        if (x >= interpolator.getX()[sustainIndex]) {
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
            currentTime = exp(interpolator.getX()[sustainIndex]) - 1;
        } else {
            level = interpolator.getY()[sustainIndex];
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

bool Envelope::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const InterpolatorProcessor::InterpolatorEvent *event_ = dynamic_cast<const InterpolatorProcessor::InterpolatorEvent*>(event)) {
        InterpolatorProcessor::processInterpolatorEvent(&interpolator, event_);
        return true;
    } else if (const ChangeSustainPositionEvent *event_ = dynamic_cast<const ChangeSustainPositionEvent*>(event)) {
        setSustainIndex(event_->sustainIndex);
        return true;
    }
    return false;
}
