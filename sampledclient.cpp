#include "sampledclient.h"
#include "jack/midiport.h"

SampledClient::SampledClient(const QString &clientName, Sampled *sampled_, bool hasMidiInput_) :
    JackClient(clientName),
    sampled(sampled_),
    hasMidiInput(hasMidiInput_),
    inputPorts(sampled->getNrOfInputs()),
    outputPorts(sampled->getNrOfOutputs()),
    inputBuffers(sampled->getNrOfInputs()),
    outputBuffers(sampled->getNrOfOutputs()),
    inputs(sampled->getNrOfInputs()),
    outputs(sampled->getNrOfOutputs())
{
}

SampledClient::~SampledClient()
{
    close();
}

bool SampledClient::init()
{
    bool ok = true;
    // create audio input and output ports:
    for (int i = 0; i < sampled->getNrOfInputs(); i++) {
        ok = ok && (inputPorts[i] = registerAudioPort(QString("audio in %1").arg(i), JackPortIsInput));
    }
    for (int i = 0; i < sampled->getNrOfOutputs(); i++) {
        ok = ok && (outputPorts[i] = registerAudioPort(QString("audio out %1").arg(i), JackPortIsOutput));
    }
    if (hasMidiInput) {
        ok = ok && (midiInputPort = registerMidiPort(QString("midi in"), JackPortIsInput));
    }
    sampled->setSampleRate(getSampleRate());
    return ok;
}

bool SampledClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    void *midiInputBuffer = (hasMidiInput ? jack_port_get_buffer(midiInputPort, nframes) : 0);
    for (int i = 0; i < inputs.size(); i++) {
        inputBuffers[i] = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(inputPorts[i], nframes));
    }
    for (int i = 0; i < outputs.size(); i++) {
        outputBuffers[i] = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(outputPorts[i], nframes));
    }
    jack_nframes_t currentFrame = 0;
    jack_nframes_t currentMidiEventIndex = 0;
    jack_nframes_t midiEventCount = (hasMidiInput ? jack_midi_get_event_count(midiInputBuffer) : 0);
    for (; currentFrame < nframes; ) {
        // get the next midi event, if there is any:
        if (currentMidiEventIndex < midiEventCount) {
            jack_midi_event_t midiEvent;
            jack_midi_event_get(&midiEvent, midiInputBuffer, currentMidiEventIndex);
            // produce audio until the event happens:
            for (; currentFrame < midiEvent.time; currentFrame++) {
                for (int i = 0; i < inputs.size(); i++) {
                    inputs[i] = inputBuffers[i][currentFrame];
                }
                sampled->process(inputs.data(), outputs.data());
                for (int i = 0; i < outputs.size(); i++) {
                    outputBuffers[i][currentFrame] = outputs[i];
                }
            }
            currentMidiEventIndex++;
            processMidi(midiEvent);
        } else {
            // produce audio until the end of the buffer:
            for (; currentFrame < nframes; currentFrame++) {
                for (int i = 0; i < inputs.size(); i++) {
                    inputs[i] = inputBuffers[i][currentFrame];
                }
                sampled->process(inputs.data(), outputs.data());
                for (int i = 0; i < outputs.size(); i++) {
                    outputBuffers[i][currentFrame] = outputs[i];
                }
            }
        }
    }
    return true;

}

void SampledClient::processMidi(const jack_midi_event_t &event)
{
}
