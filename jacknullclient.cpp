#include "jacknullclient.h"

JackNullClient::JackNullClient() :
    JackClient("null")
{
    activate();
}

JackNullClient::~JackNullClient()
{
    close();
}

bool JackNullClient::init()
{
    return true;
}

bool JackNullClient::process(jack_nframes_t)
{
    return true;
}
