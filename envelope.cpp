/*
    Copyright 2011 Arne Jacobs

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "envelope.h"
#include <QtGlobal>
#include <cmath>

Envelope::Envelope(double durationInSeconds_, double sampleRate) :
    AudioProcessor(QStringList(), QStringList("Envelope out"), sampleRate),
    durationInSeconds(durationInSeconds_),
    currentTime(0),
    previousLevel(0),
    velocity(0),
    sustainIndex(1),
    startLevel(0),
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

Envelope & Envelope::operator=(const Envelope &envelope)
{
    // copy all attributes of the given envelope:
    durationInSeconds = envelope.durationInSeconds;
    sustainIndex = envelope.sustainIndex;
    interpolator = envelope.interpolator;
    return *this;
}

void Envelope::save(QDataStream &stream) const
{
    interpolator.save(stream);
    stream << sustainIndex;
}

void Envelope::load(QDataStream &stream)
{
    interpolator.load(stream);
    int sustainIndex_;
    stream >> sustainIndex_;
    setSustainIndex(sustainIndex_);
}

Interpolator * Envelope::getInterpolator()
{
    return &interpolator;
}

void Envelope::copyInterpolatorFrom(const Envelope *envelope)
{
    interpolator = envelope->interpolator;
}

int Envelope::getSustainIndex() const
{
    return sustainIndex;
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
    startLevel = previousLevel * this->velocity / newVelocity;
    currentTime = 0.0;
    currentPhase = ATTACK;
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
        } else if (x < interpolator.getX()[1]) {
            // this is the first segment of the envelope,
            // interpolate from startLevel to segment end level instead of from 0 to segment end level:
            double endLevel = interpolator.getY()[1];
            level = interpolator.evaluate(x) * (endLevel - startLevel) / endLevel +  startLevel;
        } else {
            level = interpolator.evaluate(x);
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

bool Envelope::processEvent(const RingBufferEvent *event, jack_nframes_t)
{
    if (const InterpolatorProcessor::InterpolatorEvent *event_ = dynamic_cast<const InterpolatorProcessor::InterpolatorEvent*>(event)) {
        InterpolatorProcessor::processInterpolatorEvent(&interpolator, event_);
        if (sustainIndex >= interpolator.getX().size()) {
            setSustainIndex(interpolator.getX().size() - 1);
        }
        return true;
    } else if (const ChangeSustainPositionEvent *event_ = dynamic_cast<const ChangeSustainPositionEvent*>(event)) {
        setSustainIndex(event_->sustainIndex);
        return true;
    }
    return false;
}
