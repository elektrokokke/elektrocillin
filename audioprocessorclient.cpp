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

#include "audioprocessorclient.h"
#include "metajack/midiport.h"

AudioProcessorClient::AudioProcessorClient(const QString &clientName, AudioProcessor *audioProcessor_) :
    JackClient(clientName),
    audioProcessor(audioProcessor_),
    inputPortNames(audioProcessor->getInputPortNames()),
    outputPortNames(audioProcessor->getOutputPortNames()),
    inputPorts(inputPortNames.size()),
    outputPorts(outputPortNames.size()),
    inputBuffers(inputPortNames.size()),
    outputBuffers(outputPortNames.size()),
    inputs(inputPortNames.size()),
    outputs(outputPortNames.size())
{
}

AudioProcessorClient::AudioProcessorClient(const QString &clientName, const QStringList &inputPortNames_, const QStringList &outputPortNames_) :
    JackClient(clientName),
    audioProcessor(0),
    inputPortNames(inputPortNames_),
    outputPortNames(outputPortNames_),
    inputPorts(inputPortNames.size()),
    outputPorts(outputPortNames.size()),
    inputBuffers(inputPortNames.size()),
    outputBuffers(outputPortNames.size()),
    inputs(inputPortNames.size()),
    outputs(outputPortNames.size())
{
}

AudioProcessorClient::~AudioProcessorClient()
{
    close();
}

AudioProcessor * AudioProcessorClient::getAudioProcessor()
{
    return audioProcessor;
}

bool AudioProcessorClient::init()
{
    bool ok = true;
    // create audio input and output ports:
    for (int i = 0; ok && (i < inputPortNames.size()); i++) {
        ok = ok && (inputPorts[i] = registerAudioPort(inputPortNames[i], JackPortIsInput));
    }
    for (int i = 0; ok && (i < outputPortNames.size()); i++) {
        ok = ok && (outputPorts[i] = registerAudioPort(outputPortNames[i], JackPortIsOutput));
    }
    if (audioProcessor) {
        audioProcessor->setSampleRate(getSampleRate());
    }
    return ok;
}

bool AudioProcessorClient::process(jack_nframes_t nframes)
{
    getPortBuffers(nframes);
    processAudio(0, nframes);
    return true;
}

void AudioProcessorClient::processAudio(jack_nframes_t start, jack_nframes_t end)
{
    if (inputs.size() || outputs.size()) {
        for (jack_nframes_t currentFrame = start; currentFrame < end; currentFrame++) {
            for (int i = 0; i < inputs.size(); i++) {
                inputs[i] = inputBuffers[i][currentFrame];
            }
            processAudio(inputs.data(), outputs.data(), currentFrame);
            for (int i = 0; i < outputs.size(); i++) {
                outputBuffers[i][currentFrame] = outputs[i];
            }
        }
    }
}

void AudioProcessorClient::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    Q_ASSERT(audioProcessor);
    audioProcessor->processAudio(inputs, outputs, time);
}

void AudioProcessorClient::getPortBuffers(jack_nframes_t nframes)
{
    // get port buffers:
    for (int i = 0; i < inputs.size(); i++) {
        inputBuffers[i] = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(inputPorts[i], nframes));
    }
    for (int i = 0; i < outputs.size(); i++) {
        outputBuffers[i] = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(outputPorts[i], nframes));
    }
}
