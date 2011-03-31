#include "graphicsclientitemsclient.h"
#include "jackcontextgraphicsscene.h"
#include "metajack/recursivejackcontext.h"

QSettings GraphicsClientItemsClient::settings("settings.ini", QSettings::IniFormat);

GraphicsClientItemsClient::GraphicsClientItemsClient(QGraphicsScene *scene_) :
    JackClient("GraphicsClientItemsClient"),
    scene(scene_),
    clientStyle(3),
    audioPortStyle(1),
    midiPortStyle(3),
    font("Helvetica", 12),
    contextName(RecursiveJackContext::getInstance()->getCurrentContext()->get_name())
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
    QObject::connect(this, SIGNAL(clientRegistered(QString)), this, SLOT(onClientRegistered(QString)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(clientUnregistered(QString)), this, SLOT(onClientUnregistered(QString)), Qt::QueuedConnection);
    // the same for ports:
    QObject::connect(this, SIGNAL(portRegistered(QString,QString,int)), this, SLOT(onPortRegistered(QString,QString,int)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(portUnregistered(QString,QString,int)), this, SLOT(onPortRegistered(QString,QString,int)), Qt::QueuedConnection);
}

GraphicsClientItemsClient::~GraphicsClientItemsClient()
{
    // save all client item positions which are not JackClient client items to the settings:
    for (QMap<QString, QPointF>::iterator i = clientItemPositionMap.begin(); i != clientItemPositionMap.end(); i++) {
        settings.setValue("position/" + contextName + "/" + i.key(), i.value().toPoint());
    }
    // delete all client graphics items:
    for (QMap<QString, GraphicsClientItem*>::iterator i = clientItems.begin(); i != clientItems.end(); i++) {
        delete i.value();
    }
    // delete all port connection items:
    for (; portConnectionItems.size(); ) {
        deletePortConnectionItem(portConnectionItems.begin().key(), portConnectionItems.begin().value().begin().key());
    }
    close();
}

void GraphicsClientItemsClient::saveState(QDataStream &stream)
{
    // save client graphics positions:
    stream << clientItemPositionMap;
    // save the clients' states:
    RecursiveJackContext::getInstance()->saveCurrentContext(stream, JackClientSerializer::getInstance());
}

void GraphicsClientItemsClient::loadState(QDataStream &stream)
{
    // delete all clients and the corresponding graphics:
    QStringList clientNames = clientItems.keys();
    for (int i = 0; i < clientNames.size(); i++) {
        QString clientName = clientNames[i];
        deleteClient(clientName);
    }
    // read client graphics positions:
    stream >> clientItemPositionMap;
    // set the positions of already existing graphics items:
    for (QMap<QString, QPointF>::iterator i = clientItemPositionMap.begin(); i != clientItemPositionMap.end(); i++) {
        GraphicsClientItem *clientItem = clientItems.value(i.key(), 0);
        if (clientItem) {
            clientItem->setPos(i.value());
        }
    }
    // read the clients' and their states:
    RecursiveJackContext::getInstance()->loadCurrentContext(stream, JackClientSerializer::getInstance());
}

void GraphicsClientItemsClient::setClientStyle(int style)
{
    clientStyle = style;
}

void GraphicsClientItemsClient::setAudioPortStyle(int style)
{
    audioPortStyle = style;
}

void GraphicsClientItemsClient::setMidiPortStyle(int style)
{
    midiPortStyle = style;
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

void GraphicsClientItemsClient::deletePortConnectionItems(QString port)
{
    QStringList otherPorts = portConnectionItems.value(port).keys();
    for (int i = 0; i < otherPorts.size(); i++) {
        deletePortConnectionItem(port, otherPorts[i]);
    }
}

void GraphicsClientItemsClient::setPositions(const QString &port, const  QPointF &point)
{
    QMap<QString, GraphicsPortConnectionItem*> portItems = portConnectionItems.value(port);
    for (QMap<QString, GraphicsPortConnectionItem*>::const_iterator i = portItems.begin(); i != portItems.end(); i++) {
        i.value()->setPos(port, point);
    }
}

void GraphicsClientItemsClient::setClientItemPositionByName(const QString &clientName, QPointF pos)
{
    clientItemPositionMap[clientName] = pos;
}

void GraphicsClientItemsClient::onClientRegistered(const QString &clientName)
{
    // create a client item with that name:
    GraphicsClientItem *clientItem = 0;
    jack_client_t *client = meta_jack_client_by_name(clientName.toAscii().data());
    JackClient *jackClient = (client ? JackClientSerializer::getInstance()->getClient(client) : 0);
    if (jackClient) {
        clientItem = jackClient->createClientItem(this, clientStyle, audioPortStyle, midiPortStyle, font);
        clientItem->setSelected(true);
    } else {
        bool isMacro = RecursiveJackContext::getInstance()->getContextByClientName(clientName.toAscii().data());
        clientItem = new GraphicsClientItem(this, 0, isMacro, clientName, clientStyle, audioPortStyle, midiPortStyle, font, 0);
        QPointF pos;
        if (clientItemPositionMap.contains(clientName)) {
            pos = clientItemPositionMap[clientName];
        } else {
            pos = settings.value("position/" + contextName + "/" + clientName).toPoint();
            clientItemPositionMap.insert(clientName, pos);
        }
        clientItem->setPos(pos);
    }
    clientItems.insert(clientName, clientItem);
}

void GraphicsClientItemsClient::onClientUnregistered(const QString &clientName)
{
    // delete the client item with the given name:
    delete clientItems.value(clientName, 0);
    clientItems.remove(clientName);
    clientItemPositionMap.remove(clientName);
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

