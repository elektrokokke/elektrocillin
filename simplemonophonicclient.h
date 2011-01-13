#ifndef SIMPLEMONOPHONICCLIENT_H
#define SIMPLEMONOPHONICCLIENT_H

#include "midi2audioclient.h"
#include "oscillator.h"

class SimpleMonophonicClient : public Midi2AudioClient
{
public:
    SimpleMonophonicClient(const QString &clientName);

protected:
    virtual bool setup();
    virtual bool process(jack_nframes_t nframes);

private:
    Oscillator oscillator;
};

#endif // SIMPLEMONOPHONICCLIENT_H
