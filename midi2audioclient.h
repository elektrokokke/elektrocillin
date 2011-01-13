#ifndef MIDI2AUDIOCLIENT_H
#define MIDI2AUDIOCLIENT_H

#include "jackclient.h"

class Midi2AudioClient : public JackClient
{
public:
    Midi2AudioClient(const QString &clientName);
    Midi2AudioClient(const QString &clientName, const QString &inputPortName, const QString &outputPortName);

protected:
    virtual bool setup();

    jack_port_t *getMidiInputPort() const;
    jack_port_t *getAudioOutputPort() const;

private:
    QString midiInputPortName, audioOutputPortName;
    jack_port_t *midiInputPort, *audioOutputPort;

};

#endif // MIDI2AUDIOCLIENT_H
