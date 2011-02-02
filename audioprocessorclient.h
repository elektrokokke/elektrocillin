#ifndef SAMPLEDCLIENT_H
#define SAMPLEDCLIENT_H

#include "jackclient.h"
#include "audioprocessor.h"
#include "jack/jack.h"
#include "jack/midiport.h"
#include <QVector>

class AudioProcessorClient : public JackClient
{
public:
    AudioProcessorClient(const QString &clientName, AudioProcessor *audioProcessor);
    virtual ~AudioProcessorClient();

protected:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);
    virtual void processAudio(jack_nframes_t start, jack_nframes_t end);

    void getPortBuffers(jack_nframes_t nframes);

private:
    AudioProcessor *audioProcessor;
    QVector<jack_port_t*> inputPorts, outputPorts;
    QVector<jack_default_audio_sample_t*> inputBuffers, outputBuffers;
    QVector<double> inputs, outputs;
};

#endif // SAMPLEDCLIENT_H
