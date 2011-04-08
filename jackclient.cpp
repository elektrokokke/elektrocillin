/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jackclient.h"
#include "graphicsclientitemsclient.h"
#include <QSet>
#include <QRegExp>

JackClient::JackClient(const QString &clientName) :
    requestedName(clientName),
    actualName(clientName),
    client(0),
    processCallback(true),
    portCallbacks(false),
    clientCallback(false),
    clientItemPosition(0, 0),
    clientItemVisible(false)
{
}

JackClient::~JackClient()
{
}

JackClientFactory * JackClient::getFactory()
{
    return 0;
}

void JackClient::saveState(QDataStream &stream)
{
    stream << clientItemPosition << clientItemVisible;
}

void JackClient::loadState(QDataStream &stream)
{
    stream >> clientItemPosition >> clientItemVisible;
}

void JackClient::setCallProcess(bool processCallback)
{
    this->processCallback = processCallback;
}

void JackClient::setEmitPortSignals(bool portCallbacks)
{
    this->portCallbacks = portCallbacks;
}

void JackClient::setEmitClientSignals(bool clientCallback)
{
    this->clientCallback = clientCallback;
}

jack_client_t * JackClient::getClient()
{
    return client;
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
    if (processCallback) {
        // register the process callback:
        if (jack_set_process_callback(client, process, this)) {
            // registering process callback failed:
            jack_client_close(client);
            client = 0;
            return false;
        }
    }
    if (portCallbacks) {
        // register the port connect and register callback:
        jack_set_port_connect_callback(client, portConnectCallback, this);
        jack_set_port_registration_callback(client, portRegistrationCallback, this);
    }
    if (clientCallback) {
        jack_set_client_registration_callback(client, clientRegistrationCallback, this);
    }
    // setup input and output ports:
    if (!init()) {
        jack_client_close(client);
        client = 0;
        return false;
    }
    // associate the client handle with a pointer to this object:
    JackClientSerializer::getInstance()->registerClient(client, this);
    // activate the client:
    if (jack_activate(client)) {
        // activating client failed:
        jack_client_close(client);
        JackClientSerializer::getInstance()->unregisterClient(client);
        client = 0;
        return false;
    }
    // get the actual client name:
    actualName = jack_get_client_name (client);
    return true;
}

void JackClient::close()
{
    // close the Jack client:
    if (isActive() && (jack_client_close(client) == 0)) {
        // deassociate the client handle from the pointer to this object:
        JackClientSerializer::getInstance()->unregisterClient(client);
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

int JackClient::getPortFlags(const QString &fullPortName)
{
    return jack_port_flags(jack_port_by_name(client, fullPortName.toAscii().data()));
}

QStringList JackClient::getMyPorts(const char *typeNamePattern, unsigned long flags)
{
    return getPorts(QString(QRegExp::escape(getClientName()) + ":.*").toAscii().data(), typeNamePattern, flags);
}

QStringList JackClient::getPorts(const char *fullPortNamePattern, const char *typeNamePattern, unsigned long flags)
{
    QStringList portList;
    const char **ports = jack_get_ports(client, fullPortNamePattern, typeNamePattern, flags);
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
        connectPorts(connection[0], connection[1]);
    }
}

QStringList JackClient::getClients()
{
    QSet<QString> clientNamesSet;
    // get all ports:
    QStringList portNames = getPorts();
    for (int i = 0; i < portNames.size(); i++) {
        QString clientName = portNames[i].split(":")[0];
        clientNamesSet.insert(clientName);
    }
    QStringList clientNames = clientNamesSet.toList();
    return clientNames;
}

QString JackClient::getPortNameById(jack_port_id_t id)
{
    return jack_port_name(jack_port_by_id(client, id));
}

void JackClient::setClientItemPosition(QPointF pos)
{
    clientItemPosition = pos;
}

void JackClient::setClientItemVisible(bool visible)
{
    clientItemVisible = visible;
}

GraphicsClientItem * JackClient::createClientItem(GraphicsClientItemsClient *clientItemsClient, int clientStyle, int audioPortStyle, int midiPortStyle, QFont font)
{
    GraphicsClientItem *clientItem = new GraphicsClientItem(clientItemsClient, this, false, getClientName(), clientStyle, audioPortStyle, midiPortStyle, font, 0);
    clientItem->setPos(clientItemPosition);
    clientItem->setControlsVisible(clientItemVisible);
    return clientItem;
}

QGraphicsItem * JackClient::createGraphicsItem()
{
    return 0;
}

QString JackClient::getFullPortName(const QString &clientName, const QString &shortPortName)
{
    return clientName + ":" + shortPortName;
}

int JackClient::getMaximumPortNameLength()
{
    return jack_port_name_size();
}

void JackClient::play()
{
    jack_transport_start(client);
}

void JackClient::stop()
{
    jack_transport_stop(client);
}

void JackClient::rewind()
{
    jack_transport_locate(client, 0);
}

bool JackClient::init()
{
    return true;
}

void JackClient::deinit()
{
}

bool JackClient::process(jack_nframes_t)
{
    return true;
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

void JackClient::portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void *arg)
{
    JackClient *jackClient = reinterpret_cast<JackClient*>(arg);
    // get the port names:
    QString aName = jackClient->getPortNameById(a);
    QString bName = jackClient->getPortNameById(b);
    if (connect) {
        jackClient->portConnected(aName, bName);
    } else {
        jackClient->portDisconnected(aName, bName);
    }
}

void JackClient::portRegistrationCallback(jack_port_id_t id, int registered, void *arg)
{
    JackClient *jackClient = reinterpret_cast<JackClient*>(arg);
    // get the port:
    jack_port_t *port = jack_port_by_id(jackClient->client, id);
    // get name, type and flags:
    QString fullPortName(jack_port_name(port));
    QString type(jack_port_type(port));
    int flags = jack_port_flags(port);
    // signal the new or removed port:
    if (registered) {
        jackClient->portRegistered(fullPortName, type, flags);
    } else {
        jackClient->portUnregistered(fullPortName, type, flags);
    }
}

void JackClient::clientRegistrationCallback(const char *name, int registered, void *arg)
{
    JackClient *jackClient = reinterpret_cast<JackClient*>(arg);
    if (registered) {
        jackClient->clientRegistered(name);
    } else {
        jackClient->clientUnregistered(name);
    }
}

JackClientSerializer::JackClientSerializer()
{
}

JackClientSerializer * JackClientSerializer::serializer = 0;

JackClientSerializer * JackClientSerializer::getInstance()
{
    if (serializer == 0) {
        serializer = new JackClientSerializer();
    }
    return serializer;
}

void JackClientSerializer::registerClient(jack_client_t *client, JackClient *jackClient)
{
    mapClientHandlesToJackClients[client] = jackClient;
}

void JackClientSerializer::unregisterClient(jack_client_t *client)
{
    mapClientHandlesToJackClients.remove(client);
}

JackClient * JackClientSerializer::getClient(jack_client_t *client)
{
    return mapClientHandlesToJackClients.value(client, 0);
}

JackClient * JackClientSerializer::createClient(const QString &factoryName, const QString &clientName)
{
    // get the right factory:
    JackClientFactory *factory = getFactoryByName(factoryName);
    if (factory) {
        // create the client:
        return factory->createClient(clientName);
    } else {
        return 0;
    }
}

void JackClientSerializer::save(jack_client_t *client, QDataStream &stream)
{
    // get the associated JackClient:
    JackClient *jackClient = getClient(client);
    if (jackClient && jackClient->getFactory()) {
        stream << true;
        // save the factory name:
        stream << jackClient->getFactory()->getName();
        // save the client's state:
        jackClient->saveState(stream);
    } else {
        stream << false;
    }
}

jack_client_t * JackClientSerializer::load(const QString &clientName, QDataStream &stream)
{
    // determine wether the client can be created by us:
    bool hasFactory;
    stream >> hasFactory;
    if (hasFactory) {
        // load the factory name:
        QString factoryName;
        stream >> factoryName;
        // get the factory:
        JackClientFactory *factory = getFactoryByName(factoryName);
        // create a client:
        JackClient *jackClient = factory->createClient(clientName);
        // load it's state:
        jackClient->loadState(stream);
        // activate it:
        jackClient->activate();
        return jackClient->getClient();
    } else {
        return 0;
    }
}

void JackClientSerializer::registerFactory(JackClientFactory *factory)
{
    factories.insert(factory->getName(), factory);
}

JackClientFactory * JackClientSerializer::getFactoryByName(const QString &name)
{
    return factories.value(name, 0);
}

QList<JackClientFactory*> JackClientSerializer::getFactories()
{
    return factories.values();
}
