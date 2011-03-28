#include "graphicsclientitemsclient.h"
#include "jackcontextgraphicsscene.h"
#include "metajack/recursivejackcontext.h"

GraphicsClientItemsClient::GraphicsClientItemsClient(QGraphicsScene *scene_) :
    JackClient("GraphicsClientItemsClient"),
    scene(scene_),
    clientStyle(3),
    portStyle(3),
    font("Helvetica", 12)
{
    setCallProcess(false);
    setEmitPortSignals(true);
    setEmitClientSignals(true);
    activate();
    // get all clients and create visual representations for them:
    QStringList clientNames = getClients();
    for (int i = 0; i < clientNames.size(); i++) {
        onClientRegistered(clientNames[i]);
    }
    // make sure we're notified when clients are registered or unregistered:
    QObject::connect(this, SIGNAL(clientRegistered(QString)), this, SLOT(onClientRegistered(QString)));
    QObject::connect(this, SIGNAL(clientUnregistered(QString)), this, SLOT(onClientUnregistered(QString)));
    // the same for ports:
    QObject::connect(this, SIGNAL(portRegistered(QString,QString,int)), this, SLOT(onPortRegistered(QString,QString,int)));
    QObject::connect(this, SIGNAL(portUnregistered(QString,QString,int)), this, SLOT(onPortRegistered(QString,QString,int)));
}

GraphicsClientItemsClient::~GraphicsClientItemsClient()
{
    clear();
    close();
}

void GraphicsClientItemsClient::saveState(QDataStream &stream)
{
    clientItemPositionMap.clear();
    clientItemVisibleMap.clear();
    for (QMap<QString, GraphicsClientItem*>::iterator i = clientItems.begin(); i != clientItems.end(); i++) {
        GraphicsClientItem *clientItem = i.value();
        if (clientItem) {
            clientItemPositionMap[i.key()] = clientItem->pos();
            clientItemVisibleMap[i.key()] = clientItem->isControlsVisible();
        }
    }
    // save client graphics positions:
    stream << clientItemPositionMap;
    // save inner items' visibility:
    stream << clientItemVisibleMap;
    // save the clients' states:
    RecursiveJackContext::getInstance()->saveCurrentContext(stream, JackClientSerializer::getInstance());
}

void GraphicsClientItemsClient::loadState(QDataStream &stream)
{
    // first delete all current clients:
    deleteClients();
    // read client graphics positions:
    stream >> clientItemPositionMap;
    // read inner items' visibility:
    stream >> clientItemVisibleMap;
    // read the clients' and their states:
    RecursiveJackContext::getInstance()->loadCurrentContext(stream, JackClientSerializer::getInstance());
    for (QMap<QString, QPointF>::iterator i = clientItemPositionMap.begin(); i != clientItemPositionMap.end(); i++) {
        GraphicsClientItem *clientItem = clientItems.value(i.key(), 0);
        if (clientItem) {
            clientItem->setPos(i.value());
        }
    }
    for (QMap<QString, bool>::iterator i = clientItemVisibleMap.begin(); i != clientItemVisibleMap.end(); i++) {
        GraphicsClientItem *clientItem = clientItems.value(i.key(), 0);
        if (clientItem) {
            clientItem->setControlsVisible(i.value());
        }
    }
}

void GraphicsClientItemsClient::setClientStyle(int clientStyle)
{
    this->clientStyle = clientStyle;
}

void GraphicsClientItemsClient::setPortStyle(int portStyle)
{
    this->portStyle = portStyle;
}


void GraphicsClientItemsClient::clear()
{
    // delete all client graphics items:
    for (QMap<QString, GraphicsClientItem*>::iterator i = clientItems.begin(); i != clientItems.end(); i++) {
        delete i.value();
    }
    clientItems.clear();
    clientItemPositionMap.clear();
    clientItemVisibleMap.clear();
    // delete all port connection items:
    for (; portConnectionItems.size(); ) {
        deletePortConnectionItem(portConnectionItems.begin().key(), portConnectionItems.begin().value().begin().key());
    }
}

void GraphicsClientItemsClient::deleteClient(const QString &clientName)
{
    jack_client_t *client = meta_jack_client_by_name(clientName.toAscii().data());
    JackClient * jackClient;
    if (client && (jackClient = JackClientSerializer::getInstance()->getClient(client))) {
        delete jackClient;
    } else if (JackContext *context = RecursiveJackContext::getInstance()->getContextByClientName(clientName.toAscii().data())) {
        RecursiveJackContext::getInstance()->deleteContext(context);
    }
}

void GraphicsClientItemsClient::deleteClients()
{
    // delete all clients:
    QStringList clientNames = clientItems.keys();
    for (int i = 0; i < clientNames.size(); i++) {
        QString clientName = clientNames[i];
        deleteClient(clientName);
    }
}

QGraphicsScene * GraphicsClientItemsClient::getScene()
{
    return scene;
}

void GraphicsClientItemsClient::showAllInnerItems(bool visible)
{
    for (QMap<QString, GraphicsClientItem*>::iterator i = clientItems.begin(); i != clientItems.end(); i++) {
        GraphicsClientItem *clientItem = i.value();
        if (clientItem) {
            clientItem->toggleControls(visible);
        }
    }
}

GraphicsPortConnectionItem * GraphicsClientItemsClient::getPortConnectionItem(const QString &port1, const QString &port2)
{
    GraphicsPortConnectionItem *item = portConnectionItems.value(port1).value(port2, 0);
    if (!item) {
        item = new GraphicsPortConnectionItem(port1, port2, scene);
        portConnectionItems[port1][port2] = item;
        portConnectionItems[port2][port1] = item;
    }
    return item;
}

void GraphicsClientItemsClient::deletePortConnectionItem(QString port1, QString port2)
{
    GraphicsPortConnectionItem *item = portConnectionItems.value(port1).value(port2, 0);
    if (item) {
        portConnectionItems[port1].remove(port2);
        portConnectionItems[port2].remove(port1);
        delete item;
        if (portConnectionItems.value(port1).size() == 0) {
            portConnectionItems.remove(port1);
        }
        if (portConnectionItems.value(port2).size() == 0) {
            portConnectionItems.remove(port2);
        }
    }
}

void GraphicsClientItemsClient::setPositions(const QString &port, const  QPointF &point)
{
    QMap<QString, GraphicsPortConnectionItem*> portItems = portConnectionItems.value(port);
    for (QMap<QString, GraphicsPortConnectionItem*>::const_iterator i = portItems.begin(); i != portItems.end(); i++) {
        i.value()->setPos(port, point);
    }
}

void GraphicsClientItemsClient::setClientItemPosition(const QString &clientName, QPointF pos)
{
    GraphicsClientItem *clientItem = clientItems.value(clientName);
    if (clientItem) {
        clientItem->setPos(pos);
    }
}

void GraphicsClientItemsClient::onClientRegistered(const QString &clientName)
{
    // create a client item with that name:
    GraphicsClientItem *clientItem = new GraphicsClientItem(this, clientName, clientStyle, portStyle, font, 0);
    clientItems.insert(clientName, clientItem);
    if (clientItemPositionMap.contains(clientName)) {
        clientItem->setPos(clientItemPositionMap[clientName]);
    }
    if (clientItemVisibleMap.contains(clientName)) {
        clientItem->setControlsVisible(clientItemVisibleMap[clientName]);
    }
}

void GraphicsClientItemsClient::onClientUnregistered(const QString &clientName)
{
    // delete the client item with the given name:
    delete clientItems.value(clientName, 0);
    clientItems.remove(clientName);
}

void GraphicsClientItemsClient::onPortRegistered(QString fullPortName, QString type, int flags)
{
    // get the corresponding client name:
    QString clientName = fullPortName.split(":")[0];
    GraphicsClientItem *clientItem = clientItems.value(clientName, 0);
    if (clientItem) {
        clientItem->updatePorts();
    }
}

