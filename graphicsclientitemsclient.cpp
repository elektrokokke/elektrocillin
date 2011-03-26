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
    // the same for port connections:
    QObject::connect(this, SIGNAL(portConnected(QString,QString)), this, SLOT(onPortConnected(QString,QString)));
    QObject::connect(this, SIGNAL(portDisconnected(QString,QString)), this, SLOT(onPortDisconnected(QString,QString)));
}

GraphicsClientItemsClient::~GraphicsClientItemsClient()
{
    clear();
    close();
}

void GraphicsClientItemsClient::saveState(QDataStream &stream)
{
    RecursiveJackContext::getInstance()->saveCurrentContext(stream, JackClientSerializer::getInstance());
}

void GraphicsClientItemsClient::loadState(QDataStream &stream)
{
    // first delete all current clients:
    deleteClients();
    RecursiveJackContext::getInstance()->loadCurrentContext(stream, JackClientSerializer::getInstance());
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

void GraphicsClientItemsClient::showAllInnerItems(bool visible)
{
    for (QMap<QString, GraphicsClientItem*>::iterator i = clientItems.begin(); i != clientItems.end(); i++) {
        GraphicsClientItem *clientItem = i.value();
        if (clientItem) {
            clientItem->showInnerItem(visible);
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

void GraphicsClientItemsClient::onClientRegistered(const QString &clientName)
{
    // create a client item with that name:
    GraphicsClientItem *clientItem = new GraphicsClientItem(this, clientName, clientStyle, portStyle, font, 0);
    clientItems.insert(clientName, clientItem);
    scene->addItem(clientItem);
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

void GraphicsClientItemsClient::onPortConnected(QString sourcePortName, QString destPortName)
{
    QString clientName1 = sourcePortName.split(":")[0];
    QString clientName2 = destPortName.split(":")[0];
    GraphicsClientItem *clientItem = clientItems.value(clientName1, 0);
    if (clientItem) {
        clientItem->updatePorts();
    }
    if (clientName2 != clientName1) {
        clientItem = clientItems.value(clientName2, 0);
        if (clientItem) {
            clientItem->updatePorts();
        }
    }
}

void GraphicsClientItemsClient::onPortDisconnected(QString sourcePortName, QString destPortName)
{
    onPortConnected(sourcePortName, destPortName);
    // delete the graphical representation of the connection:
    deletePortConnectionItem(sourcePortName, destPortName);
}
