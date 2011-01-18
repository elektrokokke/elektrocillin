#ifndef SIMPLEMONOPHONICCLIENT_H
#define SIMPLEMONOPHONICCLIENT_H

#include "midi2audioclient.h"
#include "monophonicsynthesizer.h"

class SimpleMonophonicClient : public Midi2AudioClient
{
public:
    SimpleMonophonicClient(const QString &clientName);

protected:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

private:
    MonophonicSynthesizer synthesizer;
};

#endif // SIMPLEMONOPHONICCLIENT_H
