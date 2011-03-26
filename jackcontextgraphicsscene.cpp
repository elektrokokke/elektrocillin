#include "jackcontextgraphicsscene.h"
#include "graphicsportconnectionitem.h"

JackContextGraphicsScene::JackContextGraphicsScene() :
    graphicsClientItemsClient(this)
{
    setBackgroundBrush(QBrush(QColor("lightsteelblue")));
//    // get all clients and create visual representations for them:
//    QStringList clientNames = nullClient.getClients();
//    for (int i = 0; i < clientNames.size(); i++) {
//        addClient(clientNames[i]);
//    }
}

JackContextGraphicsScene::~JackContextGraphicsScene()
{
    // make sure all items are deleted before nullClient is deleted:
    // (some GraphicsPortItem instances unregister their port callbacks at nullClient when they are deleted)
    clear();
}

//GraphicsClientItem * JackContextGraphicsScene::addClient(JackClient *client)
//{
//    // activate the Jack client:
//    client->activate();
//    QString clientName = client->getClientName();
//    // create a visual representation in the scene:
//    GraphicsClientItem *graphicsClientItem = new GraphicsClientItem(client, clientStyle, portStyle, font, 0, this);
//    clientsMap.insert(client->getClientName(), QPair<JackClient*, GraphicsClientItem*>(client, graphicsClientItem));
//    return graphicsClientItem;
//}

//GraphicsClientItem * JackContextGraphicsScene::addClient(const QString &clientName)
//{
//    // try to get a jack_client_t pointer for this client:
//    jack_client_t *client = meta_jack_client_by_name(clientName.toAscii().data());
//    JackClient * jackClient;
//    if (client && (jackClient = JackClientSerializer::getInstance()->getClient(client))) {
//        return addClient(jackClient);
//    } else {
//        // create a visual representation in the scene:
//        GraphicsClientItem *graphicsClientItem = new GraphicsClientItem(&nullClient, clientName, clientStyle, portStyle, font, 0, this);
//        clientsMap.insert(clientName, QPair<JackClient*, GraphicsClientItem*>(0, graphicsClientItem));
//        return graphicsClientItem;
//    }
//}

void JackContextGraphicsScene::deleteClient(const QString &clientName)
{
    graphicsClientItemsClient.deleteClient(clientName);
//    // determine if it is a JackClient:
//    QMap<QString, QPair<JackClient*, GraphicsClientItem*> >::iterator find = clientsMap.find(clientName);
//    if (find != clientsMap.end()) {
//        // remove all connections:
//        QStringList fullPortNames = nullClient.getPorts(QString("%1:.*").arg(clientName).toAscii().data());
//        for (int i = 0; i < fullPortNames.size(); i++) {
//            deletePortConnectionItems(fullPortNames[i]);
//        }
//        if (find.value().first) {
//            JackClient *client = find.value().first;
//            delete client;
//            // determine if the given client is a macro:
//        } else if (JackContext *context = RecursiveJackContext::getInstance()->getContextByClientName(clientName.toAscii().data())) {
//            RecursiveJackContext::getInstance()->deleteContext(context);
//        } else {
//            return;
//        }
//        GraphicsClientItem *graphicsClientItem = find.value().second;
//        delete graphicsClientItem;
//        clientsMap.erase(find);
//    }
}

void JackContextGraphicsScene::saveSession(QDataStream &stream)
{
    graphicsClientItemsClient.saveState(stream);
//    RecursiveJackContext::getInstance()->saveCurrentContext(stream, JackClientSerializer::getInstance());
}

void JackContextGraphicsScene::loadSession(QDataStream &stream)
{
    graphicsClientItemsClient.loadState(stream);
//    // first delete all current clients:
//    graphicsClientItemsClient.deleteClients();
//    graphicsClientItemsClient.clear();
//    RecursiveJackContext::getInstance()->loadCurrentContext(stream, JackClientSerializer::getInstance());
//    // get all clients and create visual representations for them:
//    QStringList clientNames = nullClient.getClients();
//    for (int i = 0; i < clientNames.size(); i++) {
//        addClient(clientNames[i]);
//    }
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

void JackContextGraphicsScene::clear()
{
    graphicsClientItemsClient.clear();
    QGraphicsScene::clear();
//    clientsMap.clear();
    portConnectionItems.clear();
}

void JackContextGraphicsScene::showAllInnerItems(bool visible)
{
    graphicsClientItemsClient.showAllInnerItems(visible);
//    for (QMap<QString, QPair<JackClient*, GraphicsClientItem*> >::iterator i = clientsMap.begin(); i != clientsMap.end(); i++) {
//        GraphicsClientItem *clientItem = i.value().second;
//        if (clientItem) {
//            clientItem->showInnerItem(visible);
//        }
//    }
}

//void JackContextGraphicsScene::deleteClients()
//{
//    for (QMap<QString, QPair<JackClient*, GraphicsClientItem*> >::iterator i = clientsMap.begin(); i != clientsMap.end(); i++) {
//        deleteClient(i.key());
//    }
//    clientsMap.clear();
//}
