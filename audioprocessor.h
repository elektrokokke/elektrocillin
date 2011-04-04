#ifndef SAMPLED_H
#define SAMPLED_H

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

#include <QVector>
#include <QStringList>
#include "metajack/metajack.h"
#include <jack/ringbuffer.h>

class AudioProcessor
{
public:
    AudioProcessor(const QStringList &inputPortNames, const QStringList &outputPortNames, double sampleRate = 44100);
    AudioProcessor(const AudioProcessor &tocopy);
    virtual ~AudioProcessor();

    const QStringList & getInputPortNames() const;
    const QStringList & getOutputPortNames() const;
    int getNrOfInputs() const;
    int getNrOfOutputs() const;

    virtual void setSampleRate(double sampleRate);
    double getSampleRate() const;
    double getSampleDuration() const;
    double convertHertzToRadians(double hertz) const;

    double processAudio0(jack_nframes_t time);
    double processAudio1(double input, jack_nframes_t time);
    double processAudio2(double input1, double input2, jack_nframes_t time);

    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time) = 0;

private:
    double sampleRate, sampleDuration;
    QStringList inputPortNames, outputPortNames;
    QVector<double> inputs, outputs;
};

#endif // SAMPLED_H
