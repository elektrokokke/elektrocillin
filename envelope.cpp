/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

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

Envelope::Envelope(double durationInSeconds_) :
    AudioProcessor(QStringList(), QStringList("Envelope out")),
    LogarithmicInterpolator(0.01),
    durationInSeconds(durationInSeconds_),
    currentTime(0),
    previousLevel(0),
    velocity(0),
    sustainIndex(1),
    startLevel(0)
{
    // initialize the interpolators to represent a simple ASR envelope:
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(0);
    xx.append(log(0.5 + 1));
    yy.append(1);
    xx.append(log(durationInSeconds + 1));
    yy.append(0);
    LogarithmicInterpolator::changeControlPoints(xx, yy);
    LogarithmicInterpolator::setControlPointName(sustainIndex, "Sustain");
    LogarithmicInterpolator::setEndPointConstraints(false, true);
    // register numeric parameters:
    registerParameter("Base", 0.01, 0.001, 1, 0.001);
    registerParameter("Y steps", 0, 0, 12, 1);
}

Envelope & Envelope::operator=(const Envelope &envelope)
{
    // copy all attributes of the given envelope:
    Interpolator::changeControlPoints(envelope.xx, envelope.yy);
    durationInSeconds = envelope.durationInSeconds;
    sustainIndex = envelope.sustainIndex;
    return *this;
}

void Envelope::save(QDataStream &stream) const
{
    LogarithmicInterpolator::save(stream);
    stream << sustainIndex;
}

void Envelope::load(QDataStream &stream)
{
    LogarithmicInterpolator::load(stream);
    int sustainIndex_;
    stream >> sustainIndex_;
    setSustainIndex(sustainIndex_);
}

int Envelope::getSustainIndex() const
{
    return sustainIndex;
}

void Envelope::setSustainIndex(int sustainIndex)
{
    if ((sustainIndex > 0) && (sustainIndex < LogarithmicInterpolator::getX().size()) && (sustainIndex != this->sustainIndex)) {
        LogarithmicInterpolator::setControlPointName(this->sustainIndex, QString());
        this->sustainIndex = sustainIndex;
        LogarithmicInterpolator::setControlPointName(this->sustainIndex, "Sustain");
    }
}

double Envelope::getDurationInSeconds() const
{
    return durationInSeconds;
}

void Envelope::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char velocity, jack_nframes_t)
{
    double newVelocity = velocity / 127.0;
    startLevel = previousLevel * this->velocity / newVelocity;
    currentTime = 0.0;
    currentPhase = ATTACK;
    this->velocity = newVelocity;
    release = false;
    this->noteNumber = noteNumber;
}

void Envelope::processNoteOff(unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t)
{
    if (noteNumber == this->noteNumber) {
        release = true;
    }
}

void Envelope::processAudio(const double *, double *outputs, jack_nframes_t)
{
    double level = 0.0;
    if (currentPhase == ATTACK) {
        double x = log(currentTime + 1);
        if (x >= LogarithmicInterpolator::getX()[sustainIndex]) {
            currentPhase = SUSTAIN;
        } else if (x < LogarithmicInterpolator::getX()[1]) {
            // this is the first segment of the envelope,
            // interpolate from startLevel to segment end level instead of from 0 to segment end level:
            double endLevel = LogarithmicInterpolator::getY()[1];
            level = LogarithmicInterpolator::evaluate(x) * (endLevel - startLevel) / endLevel +  startLevel;
        } else {
            level = LogarithmicInterpolator::evaluate(x);
        }
    }
    if (currentPhase == SUSTAIN) {
        if (release) {
            currentPhase = RELEASE;
            currentTime = exp(LogarithmicInterpolator::getX()[sustainIndex]) - 1;
        } else {
            level = LogarithmicInterpolator::getY()[sustainIndex];
        }
    }
    if (currentPhase == RELEASE) {
        double x = log(currentTime + 1);
        if (x >= LogarithmicInterpolator::getX().last()) {
            currentPhase = NONE;
        } else {
            level = LogarithmicInterpolator::evaluate(x);
        }
    }
    currentTime += getSampleDuration();
    previousLevel = level;
    outputs[0] = level * velocity;
}

bool Envelope::processEvent(const RingBufferEvent *event, jack_nframes_t)
{
    if (const InterpolatorProcessor::ChangeControlPointEvent *event_ = dynamic_cast<const InterpolatorProcessor::ChangeControlPointEvent*>(event)) {
        changeControlPoint(event_->index, event_->x, event_->y);
        return true;
    } else if (const InterpolatorProcessor::AddControlPointsEvent *event_ = dynamic_cast<const InterpolatorProcessor::AddControlPointsEvent*>(event)) {
        addControlPoints(event_->scaleX, event_->scaleY, event_->addAtStart, event_->addAtEnd);
        return true;
    } else if (const InterpolatorProcessor::DeleteControlPointsEvent *event_ = dynamic_cast<const InterpolatorProcessor::DeleteControlPointsEvent*>(event)) {
        deleteControlPoints(event_->scaleX, event_->scaleY, event_->deleteAtStart, event_->deleteAtEnd);
        return true;
    } else if (const ChangeSustainPositionEvent *event_ = dynamic_cast<const ChangeSustainPositionEvent*>(event)) {
        setSustainIndex(event_->sustainIndex);
        return true;
    }
    return false;
}

bool Envelope::setParameterValue(int index, double value, jack_nframes_t time)
{
    if (index == 0) {
        LogarithmicInterpolator::setBase(value);
    }
    return ParameterProcessor::setParameterValue(index, value, time);
}

void Envelope::changeControlPoint(int index, double x, double y)
{
    double ySteps = getParameter(1).value;
    if (ySteps) {
        // discretize the Y coordinate:
        y = qRound(y * ySteps) / ySteps;
    }
    LogarithmicInterpolator::changeControlPoint(index, x, y);
}

void Envelope::addControlPoints(bool scaleX, bool scaleY, bool addAtStart, bool addAtEnd)
{
    // allow only adding at the end:
    Q_ASSERT(addAtEnd && !addAtStart);
    // insert a control point between the last and the one before that:
    Q_ASSERT(xx.size() > 1);
    double x = xx.last();
    double y = yy.last();
    xx.last() = 0.5 * (xx[xx.size() - 2] + xx.last());
    // add one at the end:
    xx.append(x);
    yy.append(y);
}

void Envelope::deleteControlPoints(bool scaleX, bool scaleY, bool deleteAtStart, bool deleteAtEnd)
{
    // allow only adding at the end:
    Q_ASSERT(deleteAtEnd && !deleteAtStart);
    if (xx.size() > 3) {
        // remove the control point before the last:
        double x = xx.last();
        double y = yy.last();
        // remove one point at the end:
        xx.remove(xx.size() - 1);
        yy.remove(yy.size() - 1);
        xx.last() = x;
        yy.last() = y;
        if (sustainIndex >= LogarithmicInterpolator::getX().size()) {
            setSustainIndex(LogarithmicInterpolator::getX().size() - 1);
        }
    }
}

