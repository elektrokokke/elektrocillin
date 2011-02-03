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

QStringList JackNullClient::getPorts(unsigned long flags)
{
    QStringList portList;
    const char **ports = jack_get_ports(client, 0, 0, flags);
    if (ports) {
        for (int i = 0; ports[i]; i++) {
            portList.append(QString(ports[i]));
        }
        // free the array:
        jack_free(ports);
    }
    return portList;
}

QStringList JackNullClient::getPortConnections(const QString &portName)
{
    QStringList portList;
    // get the port with the given name:
    jack_port_t *port = jack_port_by_name(client, portName.toAscii().data());
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

QStringList JackNullClient::getAllConnections()
{
    QStringList connectionList;
    // get all output ports:
    QStringList outputPortList = getPorts(JackPortIsOutput);
    // get all connected ports:
    for (int i = 0; i < outputPortList.size(); i++) {
        QStringList connectedPortsList = getPortConnections(outputPortList[i]);
        for (int j = 0; j < connectedPortsList.size(); j++) {
            connectionList.append(outputPortList[i] + "::" + connectedPortsList[j]);
        }
    }
    return connectionList;
}

void JackNullClient::restoreConnections(const QStringList &connections)
{
    for (int i = 0; i < connections.size(); i++) {
        QStringList connection = connections[i].split("::");
        Q_ASSERT(connection.size() == 2);
        connectPorts(connection[0], connection[1]);
    }
}

bool JackNullClient::init()
{
    return true;
}

bool JackNullClient::process(jack_nframes_t)
{
    return true;
}
