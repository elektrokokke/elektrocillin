#ifndef SAMPLEDCLIENT_H
#define SAMPLEDCLIENT_H

#include "jackclient.h"
#include "sampled.h"
#include "jack/jack.h"
#include "jack/midiport.h"
#include <QVector>

class SampledClient : public JackClient
{
public:
    SampledClient(const QString &clientName, Sampled *sampled, bool hasMidiInput = false);
    virtual ~SampledClient();

protected:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

    virtual void processMidi(const jack_midi_event_t &event);

private:
    Sampled *sampled;
    bool hasMidiInput;
    QVector<jack_port_t*> inputPorts, outputPorts;
    QVector<jack_default_audio_sample_t*> inputBuffers, outputBuffers;
    QVector<double> inputs, outputs;
    jack_port_t * midiInputPort;
};

#endif // SAMPLEDCLIENT_H
