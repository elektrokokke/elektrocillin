#include "jackclient.h"

JackClient::JackClient(const QString &clientName) :
    requestedName(clientName),
    actualName(clientName),
    client(0)
{
}

const QString & JackClient::getClientName() const
{
    return actualName;
}

bool JackClient::activate()
{
    if (isActive()) {
        return false;
    }
    // register as a Jack client:
    client = jack_client_open(requestedName.toAscii().data(), JackNullOption, 0);
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
    if (!init()) {
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
    // get the actual client name:
    actualName = jack_get_client_name (client);
    return true;
}

void JackClient::close()
{
    if (isActive()) {
        // close the Jack client:
        jack_client_close(client);
        client = 0;
        actualName = requestedName;
        // notify subclasses:
        deinit();
    }
}

void JackClient::deinit()
{
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
    Q_ASSERT(isActive());
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

bool JackClient::connectPorts(const QString &sourcePortName, const QString &destPortName)
{
    int connect = jack_connect(client, sourcePortName.toAscii().data(), destPortName.toAscii().data());
    return (connect == 0);// || (connect == EEXIST);
}

bool JackClient::disconnectPorts(const QString &sourcePortName, const QString &destPortName)
{
    return (jack_disconnect(client, sourcePortName.toAscii().data(), destPortName.toAscii().data()) == 0);
}

int JackClient::process(jack_nframes_t nframes, void *arg)
{
    // convert the void* argument to a JackClient object pointer:
    JackClient *jackClient = reinterpret_cast<JackClient*>(arg);
    // call the process method of that object:
    return jackClient->process(nframes) ? 0 : 1;
}
