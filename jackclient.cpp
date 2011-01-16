#include "jackclient.h"

JackClient::JackClient(const QString &clientName) :
    name(clientName),
    client(0)
{
}

bool JackClient::activate()
{
    if (isActive()) {
        return false;
    }
    // register as a Jack client:
    client = jack_client_open(name.toAscii().data(), JackNullOption, 0);
    if (client == 0) {
        // opening client failed:
        return false;
    }
    // register the process callback:
    if (jack_set_process_callback(client, process, this)) {
        // registering process callback failed:
        jack_client_close(client);
        client = 0;
        return false;
    }
    // setup input and output ports:
    if (!setup()) {
        jack_client_close(client);
        client = 0;
        return false;
    }
    // activate the client:
    if (jack_activate(client)) {
        // activating client failed:
        jack_client_close(client);
        client = 0;
        return false;
    }
    return true;
}

void JackClient::close()
{
    // close the Jack client:
    jack_client_close(client);
    client = 0;
}

bool JackClient::isActive() const
{
    return client;
}

jack_nframes_t JackClient::getLastFrameTime()
{
    return jack_last_frame_time(client);
}

jack_nframes_t JackClient::getEstimatedCurrentTime()
{
    return jack_frame_time(client);
}

jack_port_t * JackClient::registerAudioPort(const QString &name, unsigned long flags)
{
    return jack_port_register(client, name.toAscii().data(), JACK_DEFAULT_AUDIO_TYPE, flags, 0);
}

jack_port_t * JackClient::registerMidiPort(const QString &name, unsigned long flags)
{
    return jack_port_register(client, name.toAscii().data(), JACK_DEFAULT_MIDI_TYPE, flags, 0);
}

jack_nframes_t JackClient::getSampleRate() const
{
    return jack_get_sample_rate(client);
}

int JackClient::process(jack_nframes_t nframes, void *arg)
{
    // convert the void* argument to a JackClient object pointer:
    JackClient *jackClient = reinterpret_cast<JackClient*>(arg);
    // call the process method of that object:
    return jackClient->process(nframes) ? 0 : 1;
}
