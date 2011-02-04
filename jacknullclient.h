#ifndef JACKUTILITYCLIENT_H
#define JACKUTILITYCLIENT_H

#include "jackclient.h"

class JackNullClient : public JackClient
{
public:
    JackNullClient();
    ~JackNullClient();

protected:
    bool init();
    bool process(jack_nframes_t nframes);
};

#endif // JACKUTILITYCLIENT_H
