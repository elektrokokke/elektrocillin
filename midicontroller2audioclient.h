#ifndef MIDICONTROLLER2AUDIOCLIENT_H
#define MIDICONTROLLER2AUDIOCLIENT_H

#include "midi2audioclient.h"

class MidiController2AudioClient : public Midi2AudioClient
{
public:
    MidiController2AudioClient(const QString &clientName, unsigned char channel, unsigned char controller, jack_default_audio_sample_t min = -1.0f, jack_default_audio_sample_t max = 1.0f);

    unsigned char getChannel() const;
    unsigned char getController() const;

protected:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

private:
    unsigned char channel, controller, value;
    jack_default_audio_sample_t min, max;
};

#endif // MIDICONTROLLER2AUDIOCLIENT_H
