#include "jackcontextgraphicsscene.h"

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
    int i = clients.size();
    clients.append(client);
    // activate the Jack client:
    clients[i]->activate();
    // create a visual representation in the scene:
    GraphicsClientItem2 *graphicsClientItem = new GraphicsClientItem2(clients[i], clientStyle, portStyle, font);
    clientGraphicsItems.append(graphicsClientItem);
    QGraphicsItem *graphicsItem = clients[i]->createGraphicsItem(clientsRect);
    if (graphicsItem) {
        graphicsClientItem->setInnerItem(graphicsItem);
    }
    addItem(graphicsClientItem);
    return graphicsClientItem;
}

void JackContextGraphicsScene::saveSession(QDataStream &stream)
{
    for (int i = 0; i < clients.size(); i++) {
        if (clients[i] && clients[i]->getFactory()) {
            // save the client's factory name:
            stream << clients[i]->getFactory()->getName();
        } else {
            stream << QString();
        }
        // save the client's name:
        stream << clientGraphicsItems[i]->getClientName();
        // save the client's position:
        stream << clientGraphicsItems[i]->pos();
        if (clients[i] && clients[i]->getFactory()) {
            // save the client's state:
            clients[i]->saveState(stream);
        }
    }
    // save two empty string as end token:
    stream << QString() << QString();
    // save the connections:
    stream << nullClient.getConnections();
}

bool JackContextGraphicsScene::loadSession(QDataStream &stream)
{
    QMap<QString, int> mapClientNameToIndex;
    // first delete all current clients:
    for (int i = 0; i < clients.size(); ) {
        if (clients[i] && clients[i]->getFactory()) {
            delete clientGraphicsItems[i];
            delete clients[i];
            clientGraphicsItems.remove(i);
            clients.remove(i);
        } else {
            mapClientNameToIndex[clientGraphicsItems[i]->getClientName()] = i;
            i++;
        }
    }
    // load the client names and create them:
    QString factoryName, clientName;
    stream >> factoryName >> clientName;
    for (; !factoryName.isNull() || !clientName.isNull(); ) {
        // load the client's position:
        QPointF position;
        stream >> position;
        if (factoryName.isNull()) {
            clientGraphicsItems[mapClientNameToIndex[clientName]]->setPos(position);
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

GraphicsClientItem2 * JackContextGraphicsScene::addClient(const QString &clientName)
{
    clients.append(0);
    // create a visual representation in the scene:
    GraphicsClientItem2 *graphicsClientItem = new GraphicsClientItem2(&nullClient, clientName, clientStyle, portStyle, font);
    clientGraphicsItems.append(graphicsClientItem);
    addItem(graphicsClientItem);
    return graphicsClientItem;
}
