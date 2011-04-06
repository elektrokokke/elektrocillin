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
    startLevel(0)
{
    // register numeric parameters:
    registerParameter("Base", 0.01, 0.001, 1, 0.001);
    registerParameter("Sustain index", 1, 1, getNrOfControlPoints() - 1, 1);
    registerParameter("Y steps", 0, 0, 12, 1);
    // initialize the interpolators to represent a simple ASR envelope:
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(0);
    xx.append(log(0.5 + 1));
    yy.append(1);
    xx.append(log(durationInSeconds + 1));
    yy.append(0);
    LogarithmicInterpolator::changeControlPoints(xx, yy);
    LogarithmicInterpolator::setControlPointName(getSustainIndex(), "Sustain");
    LogarithmicInterpolator::setEndPointConstraints(false, true);
    sustainIndex = getSustainIndex();
}

Envelope & Envelope::operator=(const Envelope &envelope)
{
    // copy ParameterProcessor attributes
    ParameterProcessor::operator =(envelope);
    // copy all attributes of the given envelope:
    Interpolator::changeControlPoints(envelope.xx, envelope.yy);
    durationInSeconds = envelope.durationInSeconds;
    setSustainIndex(qRound(getParameter(1).value));
    return *this;
}

void Envelope::save(QDataStream &stream) const
{
    LogarithmicInterpolator::save(stream);
}

void Envelope::load(QDataStream &stream)
{
    LogarithmicInterpolator::load(stream);
    setSustainIndex(qRound(getParameter(1).value));
}

int Envelope::getSustainIndex() const
{
    return qRound(getParameter(1).value);
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
    if (const Interpolator::InterpolatorEvent *event_ = dynamic_cast<const Interpolator::InterpolatorEvent*>(event)) {
        processInterpolatorEvent(event_);
        return true;
    } else {
        return false;
    }
}

bool Envelope::setParameterValue(int index, double value, jack_nframes_t time)
{
    if (ParameterProcessor::setParameterValue(index, value, time)) {
        if (index == 0) {
            LogarithmicInterpolator::setBase(value);
        } else if (index == 1) {
            setSustainIndex(qRound(value));
        }
        return true;
    } else {
        return false;
    }
}

void Envelope::changeControlPoint(int index, double x, double y)
{
    double ySteps = getParameter(2).value;
    if (ySteps) {
        // discretize the Y coordinate:
        y = qRound(y * ySteps) / ySteps;
    }
    LogarithmicInterpolator::changeControlPoint(index, x, y);
}

void Envelope::deleteControlPoint(int index)
{
    if (getNrOfControlPoints() > 3) {
        LogarithmicInterpolator::deleteControlPoint(index);
    }
}

void Envelope::addControlPoint(double x, double y)
{
    double ySteps = getParameter(2).value;
    if (ySteps) {
        // discretize the Y coordinate:
        y = qRound(y * ySteps) / ySteps;
    }
    LogarithmicInterpolator::addControlPoint(x, y);
}

void Envelope::controlPointsChanged()
{
    if (sustainIndex >= getNrOfControlPoints()) {
        setSustainIndex(getNrOfControlPoints() - 1);
    }
    // adjust the bounds of the sustain parameter:
    ParameterProcessor::Parameter &parameter = getParameter(1);
    parameter.max = getNrOfControlPoints() - 1;
}

void Envelope::setSustainIndex(int sustainIndex)
{
    if ((sustainIndex > 0) && (sustainIndex < getNrOfControlPoints())) {
        LogarithmicInterpolator::setControlPointName(this->sustainIndex, QString());
        LogarithmicInterpolator::setControlPointName(sustainIndex, "Sustain");
        this->sustainIndex = sustainIndex;
    }
}
