#include "midi2audioclient.h"

Midi2AudioClient::Midi2AudioClient(const QString &clientName) :
    JackClient(clientName),
    midiInputPortName("midi in"),
    audioOutputPortName("audio out")
{
}

Midi2AudioClient::Midi2AudioClient(const QString &clientName, const QString &inputPortName, const QString &outputPortName) :
    JackClient(clientName),
    midiInputPortName(inputPortName),
    audioOutputPortName(outputPortName)
{
}

bool Midi2AudioClient::init()
{
    // register input and output ports:
    midiInputPort = registerMidiPort(midiInputPortName, JackPortIsInput);
    audioOutputPort = registerAudioPort(audioOutputPortName, JackPortIsOutput);
    return midiInputPort && audioOutputPort;
}

jack_port_t * Midi2AudioClient::getMidiInputPort() const
{
    return midiInputPort;
}

jack_port_t * Midi2AudioClient::getAudioOutputPort() const
{
    return audioOutputPort;
}

