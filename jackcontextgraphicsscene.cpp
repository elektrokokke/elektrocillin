#include "jackcontextgraphicsscene.h"
#include "graphicsportconnectionitem.h"

JackContextGraphicsScene::JackContextGraphicsScene() :
    clientStyle(1),
    portStyle(3),
    font("Helvetica", 12),
    clientsRect(0, 0, 600, 420)
{
    setBackgroundBrush(QBrush(QColor("lightsteelblue")));
    // get all clients and create visual representations for them:
    QStringList clientNames = nullClient.getClients();
    for (int i = 0; i < clientNames.size(); i++) {
        addClient(clientNames[i]);
    }
}

JackContextGraphicsScene::~JackContextGraphicsScene()
{
    // make sure all items are deleted before nullClient is deleted:
    // (some GraphicsPortItem instances unregister their port callbacks at nullClient when they are deleted)
    clear();
}

void JackContextGraphicsScene::setClientStyle(int clientStyle)
{
    this->clientStyle = clientStyle;
}

void JackContextGraphicsScene::setPortStyle(int portStyle)
{
    this->portStyle = portStyle;
}

GraphicsClientItem * JackContextGraphicsScene::addClient(JackClient *client)
{
    // activate the Jack client:
    client->activate();
    // create a visual representation in the scene:
    GraphicsClientItem *graphicsClientItem = new GraphicsClientItem(client, clientStyle, portStyle, font, 0, this);
    QGraphicsItem *graphicsItem = client->createGraphicsItem(clientsRect);
    if (graphicsItem) {
        graphicsClientItem->setInnerItem(graphicsItem);
    }
    addItem(graphicsClientItem);
    clientsMap.insert(client->getClientName(), QPair<JackClient*, GraphicsClientItem*>(client, graphicsClientItem));
    return graphicsClientItem;
}

GraphicsClientItem * JackContextGraphicsScene::addClient(const QString &clientName)
{
    // try to get a jack_client_t pointer for this client:
    jack_client_t *client = meta_jack_client_by_name(clientName.toAscii().data());
    JackClient * jackClient;
    if (client && (jackClient = JackClient::getClient(client))) {
        return addClient(jackClient);
    } else {
        // create a visual representation in the scene:
        GraphicsClientItem *graphicsClientItem = new GraphicsClientItem(&nullClient, clientName, clientStyle, portStyle, font, 0, this);
        addItem(graphicsClientItem);
        clientsMap.insert(clientName, QPair<JackClient*, GraphicsClientItem*>(0, graphicsClientItem));
        return graphicsClientItem;
    }
}

void JackContextGraphicsScene::deleteClient(JackClient *client)
{
    QMap<QString, QPair<JackClient*, GraphicsClientItem*> >::iterator find = clientsMap.find(client->getClientName());
    if (find != clientsMap.end()) {
        JackClient *client = find.value().first;
        GraphicsClientItem *graphicsClientItem = find.value().second;
        QStringList fullPortNames = client->getMyPorts();
        for (int i = 0; i < fullPortNames.size(); i++) {
            deletePortConnectionItems(fullPortNames[i]);
        }
        delete graphicsClientItem;
        delete client;
        clientsMap.erase(find);
    }
}

void JackContextGraphicsScene::saveSession(QDataStream &stream)
{
    for (QMap<QString, QPair<JackClient*, GraphicsClientItem*> >::iterator i = clientsMap.begin(); i != clientsMap.end(); i++) {
        JackClient *client = i.value().first;
        GraphicsClientItem *graphicsClientItem = i.value().second;
        if (client && client->getFactory()) {
            // save the client's factory name:
            stream << client->getFactory()->getName();
        } else {
            stream << QString();
        }
        // save the client's name:
        stream << graphicsClientItem->getClientName();
        // save the client's position:
        stream << graphicsClientItem->pos();
        if (client && client->getFactory()) {
            // save the client's state:
            client->saveState(stream);
        }
    }
    // save two empty string as end token:
    stream << QString() << QString();
    // save the connections:
    stream << nullClient.getConnections();
}

bool JackContextGraphicsScene::loadSession(QDataStream &stream)
{
    // first delete all current clients:
    deleteAllClients();
    // load the client names and create them:
    QString factoryName, clientName;
    stream >> factoryName >> clientName;
    for (; !factoryName.isNull() || !clientName.isNull(); ) {
        // load the client's position:
        QPointF position;
        stream >> position;
        if (factoryName.isNull()) {
            clientsMap[clientName].second->setPos(position);
        } else {
            // get the factory associated with that name:
            JackClientFactory *factory = JackClientFactory::getFactoryByName(factoryName);
            if (!factory) {
                return false;
            }
            JackClient *client = factory->createClient(clientName);
            // load the client's state:
            client->loadState(stream);
            // show the client:
            addClient(client)->setPos(position);
        }
        stream >> factoryName >> clientName;
    }
    // load the connections and restore them:
    QStringList connections;
    stream >> connections;
    nullClient.restoreConnections(connections);
    return true;
}

GraphicsPortConnectionItem * JackContextGraphicsScene::getPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene)
{
    GraphicsPortConnectionItem *item = portConnectionItems.value(port1).value(port2, 0);
    if (!item) {
        item = new GraphicsPortConnectionItem(port1, port2, scene);
        portConnectionItems[port1][port2] = item;
        portConnectionItems[port2][port1] = item;
    }
    return item;
}

void JackContextGraphicsScene::deletePortConnectionItem(const QString &port1, const QString &port2)
{
    GraphicsPortConnectionItem *item = portConnectionItems.value(port1).value(port2, 0);
    if (item) {
        portConnectionItems[port1].remove(port2);
        portConnectionItems[port2].remove(port1);
        delete item;
        if (portConnectionItems[port1].size() == 0) {
            portConnectionItems.remove(port1);
        }
        if (portConnectionItems[port2].size() == 0) {
            portConnectionItems.remove(port2);
        }
    }
}

void JackContextGraphicsScene::setPositions(const QString &port, const  QPointF &point)
{
    const QMap<QString, GraphicsPortConnectionItem*> &portItems = portConnectionItems[port];
    for (QMap<QString, GraphicsPortConnectionItem*>::const_iterator i = portItems.begin(); i != portItems.end(); i++) {
        i.value()->setPos(port, point);
    }
}

void JackContextGraphicsScene::deletePortConnectionItems(const QString &fullPortName)
{
    QStringList otherPorts = portConnectionItems.value(fullPortName).keys();
    for (int i = 0; i < otherPorts.size(); i++) {
        deletePortConnectionItem(fullPortName, otherPorts[i]);
    }
}

void JackContextGraphicsScene::deleteAllClients()
{
    QList<JackClient*> clients;
    for (QMap<QString, QPair<JackClient*, GraphicsClientItem*> >::iterator i = clientsMap.begin(); i != clientsMap.end(); i++) {
        JackClient *client = i.value().first;
        if (client) {
            clients.append(client);
        }
    }
    for (int i = 0; i < clients.size(); i++) {
        deleteClient(clients[i]);
    }
}
