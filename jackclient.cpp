#include "jackclient.h"
#include <QDebug>

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

bool JackClient::isActive() const
{
    return client;
}

jack_nframes_t JackClient::getEstimatedCurrentTime()
{
    Q_ASSERT(isActive());
    return jack_frame_time(client);
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

QString JackClient::getPortType(const QString &fullPortName)
{
    return QString(jack_port_type(jack_port_by_name(client, fullPortName.toAscii().data())));
}

QStringList JackClient::getMyPorts(const char *typeNamePattern, unsigned long flags)
{
    return getPorts(QString("%1:.*").arg(getClientName()).toAscii().data(), typeNamePattern, flags);
}

QStringList JackClient::getPorts(const char *clientNamePattern, const char *typeNamePattern, unsigned long flags)
{
    QStringList portList;
    const char **ports = jack_get_ports(client, clientNamePattern, typeNamePattern, flags);
    if (ports) {
        for (int i = 0; ports[i]; i++) {
            portList.append(QString(ports[i]));
        }
        // free the array:
        jack_free(ports);
    }
    return portList;
}

QStringList JackClient::getConnectedPorts(const QString &fullPortName)
{
    QStringList portList;
    // get the port with the given name:
    jack_port_t *port = jack_port_by_name(client, fullPortName.toAscii().data());
    if (port) {
        const char **ports = jack_port_get_all_connections(client, port);
        if (ports) {
            for (int i = 0; ports[i]; i++) {
                portList.append(QString(ports[i]));
            }
            // free the array:
            jack_free(ports);
        }
    }
    return portList;
}

QStringList JackClient::getConnections()
{
    QStringList connectionList;
    // get all output ports:
    QStringList outputPortList = getPorts(0, 0, JackPortIsOutput);
    // get all connected ports:
    for (int i = 0; i < outputPortList.size(); i++) {
        QStringList connectedPortsList = getConnectedPorts(outputPortList[i]);
        for (int j = 0; j < connectedPortsList.size(); j++) {
            connectionList.append(outputPortList[i] + "::" + connectedPortsList[j]);
        }
    }
    return connectionList;
}

void JackClient::restoreConnections(const QStringList &connections)
{
    for (int i = 0; i < connections.size(); i++) {
        QStringList connection = connections[i].split("::");
        Q_ASSERT(connection.size() == 2);
        if (!connectPorts(connection[0], connection[1])) {
            qDebug() << "void JackClient::restoreConnections(const QStringList &connections)" << "could not restore connection" << connections[i];
        }
    }
}

QString JackClient::getFullPortName(const QString &clientName, const QString &shortPortName)
{
    return clientName + ":" + shortPortName;
}

int JackClient::getMaximumPortNameLength()
{
    return jack_port_name_size();
}

void JackClient::deinit()
{
}

int JackClient::process(jack_nframes_t nframes, void *arg)
{
    // convert the void* argument to a JackClient object pointer:
    JackClient *jackClient = reinterpret_cast<JackClient*>(arg);
    // call the process method of that object:
    return jackClient->process(nframes) ? 0 : 1;
}

jack_nframes_t JackClient::getLastFrameTime()
{
    return jack_last_frame_time(client);
}

jack_nframes_t JackClient::getSampleRate() const
{
    return jack_get_sample_rate(client);
}

jack_port_t * JackClient::registerAudioPort(const QString &name, unsigned long flags)
{
    return jack_port_register(client, name.toAscii().data(), JACK_DEFAULT_AUDIO_TYPE, flags, 0);
}

jack_port_t * JackClient::registerMidiPort(const QString &name, unsigned long flags)
{
    return jack_port_register(client, name.toAscii().data(), JACK_DEFAULT_MIDI_TYPE, flags, 0);
}
