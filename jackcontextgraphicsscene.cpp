#include "jackcontextgraphicsscene.h"
#include "graphicsportconnectionitem.h"

JackContextGraphicsScene::JackContextGraphicsScene(int clientStyle_, int portStyle_, const QFont &font_) :
    clientStyle(clientStyle_),
    portStyle(portStyle_),
    font(font_),
    clientsRect(0, 0, 600, 420)
{
    // get all clients and create visual representations for them:
    QStringList clientNames = nullClient.getClients();
    for (int i = 0; i < clientNames.size(); i++) {
        addClient(clientNames[i]);
    }
}

JackContextGraphicsScene::~JackContextGraphicsScene()
{
}

void JackContextGraphicsScene::setClientStyle(int clientStyle)
{
    this->clientStyle = clientStyle;
}

void JackContextGraphicsScene::setPortStyle(int portStyle)
{
    this->portStyle = portStyle;
}

GraphicsClientItem2 * JackContextGraphicsScene::addClient(JackClient *client)
{
    // activate the Jack client:
    client->activate();
    // create a visual representation in the scene:
    GraphicsClientItem2 *graphicsClientItem = new GraphicsClientItem2(client, clientStyle, portStyle, font);
    QGraphicsItem *graphicsItem = client->createGraphicsItem(clientsRect);
    if (graphicsItem) {
        graphicsClientItem->setInnerItem(graphicsItem);
    }
    addItem(graphicsClientItem);
    clientsMap.insert(client->getClientName(), QPair<JackClient*, GraphicsClientItem2*>(client, graphicsClientItem));
    return graphicsClientItem;
}

GraphicsClientItem2 * JackContextGraphicsScene::addClient(const QString &clientName)
{
    // try to get a jack_client_t pointer for this client:
    jack_client_t *client = meta_jack_client_by_name(clientName.toAscii().data());
    JackClient * jackClient;
    if (client && (jackClient = JackClient::getClient(client))) {
        return addClient(jackClient);
    } else {
        // create a visual representation in the scene:
        GraphicsClientItem2 *graphicsClientItem = new GraphicsClientItem2(&nullClient, clientName, clientStyle, portStyle, font);
        addItem(graphicsClientItem);
        clientsMap.insert(clientName, QPair<JackClient*, GraphicsClientItem2*>(jackClient, graphicsClientItem));
        return graphicsClientItem;
    }
}

void JackContextGraphicsScene::removeClient(JackClient *client)
{
    QMap<QString, QPair<JackClient*, GraphicsClientItem2*> >::iterator find = clientsMap.find(client->getClientName());
    if (find != clientsMap.end()) {
        JackClient *client = find.value().first;
        GraphicsClientItem2 *graphicsClientItem = find.value().second;
        QStringList fullPortNames = client->getMyPorts();
        for (int i = 0; i < fullPortNames.size(); i++) {
            GraphicsPortConnectionItem::deletePortConnectionItems(fullPortNames[i]);
        }
        delete graphicsClientItem;
        delete client;
        clientsMap.erase(find);
    }
}

void JackContextGraphicsScene::saveSession(QDataStream &stream)
{
    for (QMap<QString, QPair<JackClient*, GraphicsClientItem2*> >::iterator i = clientsMap.begin(); i != clientsMap.end(); i++) {
        JackClient *client = i.value().first;
        GraphicsClientItem2 *graphicsClientItem = i.value().second;
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
    removeAllClients();
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

void JackContextGraphicsScene::removeAllClients()
{
    QList<JackClient*> clients;
    for (QMap<QString, QPair<JackClient*, GraphicsClientItem2*> >::iterator i = clientsMap.begin(); i != clientsMap.end(); i++) {
        JackClient *client = i.value().first;
        if (client) {
            clients.append(client);
        }
    }
    for (int i = 0; i < clients.size(); i++) {
        removeClient(clients[i]);
    }
}
