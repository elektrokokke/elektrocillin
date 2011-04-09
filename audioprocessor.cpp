/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "audioprocessor.h"
#include <cmath>

AudioProcessor::AudioProcessor(const QStringList &inputPortNames_, const QStringList &outputPortNames_) :
    sampleRate(44100.0),
    sampleDuration(1.0 / sampleRate),
    audioInputPortNames(inputPortNames_),
    audioOutputPortNames(outputPortNames_),
    inputs(audioInputPortNames.size()),
    outputs(audioOutputPortNames.size())
{
}

AudioProcessor::AudioProcessor(const AudioProcessor &tocopy) :
    sampleRate(tocopy.sampleRate),
    sampleDuration(tocopy.sampleDuration),
    audioInputPortNames(tocopy.audioInputPortNames),
    audioOutputPortNames(tocopy.audioOutputPortNames),
    inputs(tocopy.inputs),
    outputs(tocopy.outputs)
{
}

AudioProcessor::~AudioProcessor()
{
}

const QStringList & AudioProcessor::getAudioInputPortNames() const
{
    return audioInputPortNames;
}
const QStringList & AudioProcessor::getAudioOutputPortNames() const
{
    return audioOutputPortNames;
}

int AudioProcessor::getNrOfAudioInputs() const
{
    return audioInputPortNames.size();
}

int AudioProcessor::getNrOfAudioOutputs() const
{
    return audioOutputPortNames.size();
}

void AudioProcessor::setSampleRate(double sampleRate)
{
    this->sampleRate = sampleRate;
    sampleDuration = 1.0 / sampleRate;
}

double AudioProcessor::getSampleRate() const
{
    return sampleRate;
}

double AudioProcessor::getSampleDuration() const
{
    return sampleDuration;
}

double AudioProcessor::convertHertzToRadians(double hertz) const
{
    return hertz * 2.0 * M_PI / getSampleRate();
}

double AudioProcessor::processAudio0(jack_nframes_t time)
{
    Q_ASSERT((inputs.size() == 0) && (outputs.size() == 1));
    processAudio(inputs.data(), outputs.data(), time);
    return outputs[0];
}

double AudioProcessor::processAudio1(double input, jack_nframes_t time)
{
    Q_ASSERT((inputs.size() == 1) && (outputs.size() == 1));
    inputs[0] = input;
    processAudio(inputs.data(), outputs.data(), time);
    return outputs[0];
}

double AudioProcessor::processAudio2(double input1, double input2, jack_nframes_t time)
{
    Q_ASSERT((inputs.size() == 2) && (outputs.size() == 1));
    inputs[0] = input1;
    inputs[1] = input2;
    processAudio(inputs.data(), outputs.data(), time);
    return outputs[0];
}
