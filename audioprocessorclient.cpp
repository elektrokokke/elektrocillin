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

#include "audioprocessorclient.h"
#include "metajack/midiport.h"

AudioProcessorClient::AudioProcessorClient(const QString &clientName, AudioProcessor *audioProcessor_) :
    JackClient(clientName),
    audioProcessor(audioProcessor_),
    audioInputPortNames(audioProcessor->getAudioInputPortNames()),
    audioOutputPortNames(audioProcessor->getAudioOutputPortNames()),
    audioInputPorts(audioInputPortNames.size()),
    audioOutputPorts(audioOutputPortNames.size()),
    audioInputBuffers(audioInputPortNames.size()),
    audioOutputBuffers(audioOutputPortNames.size()),
    inputs(audioInputPortNames.size()),
    outputs(audioOutputPortNames.size())
{
}

AudioProcessorClient::AudioProcessorClient(const QString &clientName, const QStringList &inputPortNames_, const QStringList &outputPortNames_) :
    JackClient(clientName),
    audioProcessor(0),
    audioInputPortNames(inputPortNames_),
    audioOutputPortNames(outputPortNames_),
    audioInputPorts(audioInputPortNames.size()),
    audioOutputPorts(audioOutputPortNames.size()),
    audioInputBuffers(audioInputPortNames.size()),
    audioOutputBuffers(audioOutputPortNames.size()),
    inputs(audioInputPortNames.size()),
    outputs(audioOutputPortNames.size())
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
    for (int i = 0; ok && (i < audioInputPortNames.size()); i++) {
        ok = ok && (audioInputPorts[i] = registerAudioPort(audioInputPortNames[i], JackPortIsInput));
    }
    for (int i = 0; ok && (i < audioOutputPortNames.size()); i++) {
        ok = ok && (audioOutputPorts[i] = registerAudioPort(audioOutputPortNames[i], JackPortIsOutput));
    }
    if (audioProcessor) {
        audioProcessor->setSampleRate(getSampleRate());
    }
    return ok;
}

bool AudioProcessorClient::process(jack_nframes_t nframes)
{
    getAudioPortBuffers(nframes);
    processAudio(0, nframes);
    return true;
}

void AudioProcessorClient::processAudio(jack_nframes_t start, jack_nframes_t end)
{
    if (inputs.size() || outputs.size()) {
        for (jack_nframes_t currentFrame = start; currentFrame < end; currentFrame++) {
            for (int i = 0; i < inputs.size(); i++) {
                inputs[i] = audioInputBuffers[i][currentFrame];
            }
            processAudio(inputs.data(), outputs.data(), currentFrame);
            for (int i = 0; i < outputs.size(); i++) {
                audioOutputBuffers[i][currentFrame] = outputs[i];
            }
        }
    }
}

void AudioProcessorClient::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    Q_ASSERT(audioProcessor);
    audioProcessor->processAudio(inputs, outputs, time);
}

void AudioProcessorClient::getAudioPortBuffers(jack_nframes_t nframes)
{
    // get audio port buffers:
    for (int i = 0; i < inputs.size(); i++) {
        audioInputBuffers[i] = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(audioInputPorts[i], nframes));
    }
    for (int i = 0; i < outputs.size(); i++) {
        audioOutputBuffers[i] = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(audioOutputPorts[i], nframes));
    }
}

jack_default_audio_sample_t * AudioProcessorClient::getInputBuffer(int index)
{
    Q_ASSERT((index >= 0) && (index < audioInputBuffers.size()));
    return audioInputBuffers[index];
}

jack_default_audio_sample_t * AudioProcessorClient::getOutputBuffer(int index)
{
    Q_ASSERT((index >= 0) && (index < audioInputBuffers.size()));
    return audioOutputBuffers[index];
}
