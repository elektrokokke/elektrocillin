#include "audioprocessorclient.h"
#include "jack/midiport.h"

AudioProcessorClient::AudioProcessorClient(const QString &clientName, AudioProcessor *audioProcessor_) :
    JackClient(clientName),
    audioProcessor(audioProcessor_),
    inputPorts(audioProcessor->getNrOfInputs()),
    outputPorts(audioProcessor->getNrOfOutputs()),
    inputBuffers(audioProcessor->getNrOfInputs()),
    outputBuffers(audioProcessor->getNrOfOutputs()),
    inputs(audioProcessor->getNrOfInputs()),
    outputs(audioProcessor->getNrOfOutputs())
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
    for (int i = 0; i < audioProcessor->getNrOfInputs(); i++) {
        ok = ok && (inputPorts[i] = registerAudioPort(QString("audio in %1").arg(i), JackPortIsInput));
    }
    for (int i = 0; i < audioProcessor->getNrOfOutputs(); i++) {
        ok = ok && (outputPorts[i] = registerAudioPort(QString("audio out %1").arg(i), JackPortIsOutput));
    }
    audioProcessor->setSampleRate(getSampleRate());
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
    for (jack_nframes_t currentFrame = start; currentFrame < end; currentFrame++) {
        for (int i = 0; i < inputs.size(); i++) {
            inputs[i] = inputBuffers[i][currentFrame];
        }
        audioProcessor->processAudio(inputs.data(), outputs.data());
        for (int i = 0; i < outputs.size(); i++) {
            outputBuffers[i][currentFrame] = outputs[i];
        }
    }
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
