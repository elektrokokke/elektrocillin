#include "graphicsportitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QBrush>
#include <QFont>
#include <QSet>
#include <QGraphicsScene>

GraphicsPortItem::GraphicsPortItem(JackClient *client_, const QString &fullPortName_, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    client(client_),
    clientName(client->getClientName()),
    fullPortName(fullPortName_),
    shortPortName(fullPortName.split(":")[1]),
    portNameItem(new QGraphicsSimpleTextItem(shortPortName, this))
{
    init();
}

void GraphicsPortItem::init()
{
    setBrush(QBrush(Qt::white));
    portNameItem->setFont(QFont("Helvetica", 8));
    setRect(portNameItem->boundingRect().adjusted(0, 0, 4, 2));
    portNameItem->setPos(2, 1);
    // get port info:
    type = client->getPortType(fullPortName);
    isInput = (client->getPortFlags(fullPortName) & JackPortIsInput);
    // create the context menu:
    connectMenu = contextMenu.addMenu("Connect");
    disconnectMenu = contextMenu.addMenu("Disconnect");
    QStringList connectedPorts = client->getConnectedPorts(fullPortName);
    QSet<QString> connectedPortsSet;
    for (int i = 0; i < connectedPorts.size(); i++) {
        QAction *action = disconnectMenu->addAction(connectedPorts[i]);
        action->setData(connectedPorts[i]);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onDisconnectAction()));
        mapPortNamesToActions[connectedPorts[i]] = action;
        connectedPortsSet.insert(connectedPorts[i]);
    }
    // get all available ports that can be connected to this:
    QStringList connectablePorts = client->getPorts(0, type.toAscii().data(), isInput ? JackPortIsOutput : JackPortIsInput);
    for (int i = 0; i < connectablePorts.size(); i++) {
        // skip ports that are already connected:
        if (!connectedPorts.contains(connectablePorts[i])) {
            QAction *action = connectMenu->addAction(connectablePorts[i]);
            action->setData(connectablePorts[i]);
            QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
            mapPortNamesToActions[connectablePorts[i]] = action;
        }
    }
    disconnectMenu->setEnabled(disconnectMenu->actions().size());
    connectMenu->setEnabled(connectMenu->actions().size());
    // register the port connection callback at the jack server:
    client->registerPortConnectInterface(fullPortName, this);

    setFlags(QGraphicsItem::ItemSendsGeometryChanges);
}

void GraphicsPortItem::connectedTo(const QString &otherPort)
{
    connectMenu->removeAction(mapPortNamesToActions[otherPort]);
    QAction *action = disconnectMenu->addAction(otherPort);
    action->setData(otherPort);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onDisconnectAction()));
    mapPortNamesToActions[otherPort] = action;
    disconnectMenu->setEnabled(disconnectMenu->actions().size());
    connectMenu->setEnabled(connectMenu->actions().size());
    GraphicsPortConnectionItem *connectionItem = GraphicsPortConnectionItem::getPortConnectionItem(fullPortName, otherPort, scene());
    connectionItem->setPos(fullPortName, sceneBoundingRect().center());
}

void GraphicsPortItem::disconnectedFrom(const QString &otherPort)
{
    disconnectMenu->removeAction(mapPortNamesToActions[otherPort]);
    QAction *action = connectMenu->addAction(otherPort);
    action->setData(otherPort);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
    mapPortNamesToActions[otherPort] = action;
    disconnectMenu->setEnabled(disconnectMenu->actions().size());
    connectMenu->setEnabled(connectMenu->actions().size());
    GraphicsPortConnectionItem::deletePortConnectionItem(fullPortName, otherPort);
}

void GraphicsPortItem::registeredPort(const QString &fullPortname, const QString &type, int flags)
{
    bool portIsInput = (flags & JackPortIsInput);
    if ((this->type == type) && (portIsInput != isInput)) {
        // put the port into our connection menu:
        QAction *action = connectMenu->addAction(fullPortname);
        action->setData(fullPortname);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
        mapPortNamesToActions[fullPortname] = action;
        connectMenu->setEnabled(connectMenu->actions().size());
    }
}

void GraphicsPortItem::unregisteredPort(const QString &fullPortname, const QString &type, int flags)
{
    QAction *action = mapPortNamesToActions.value(fullPortname, 0);
    if (action) {
        // remove the port from our menus:
        connectMenu->removeAction(action);
        disconnectMenu->removeAction(action);
        mapPortNamesToActions.remove(fullPortName);
        disconnectMenu->setEnabled(disconnectMenu->actions().size());
        connectMenu->setEnabled(connectMenu->actions().size());
    }
}

void GraphicsPortItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsRectItem::mousePressEvent(event);
    if (!event->isAccepted() && (event->button() == Qt::RightButton)) {
        event->accept();
        // show a menu that allows removing and adding control points:
        contextMenu.exec(event->screenPos());
    }
}

QVariant GraphicsPortItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) {
        GraphicsPortConnectionItem::setPositions(fullPortName, sceneBoundingRect().center() - pos() + value.toPointF());
    }
    return QGraphicsItem::itemChange(change, value);
}

void GraphicsPortItem::onConnectAction()
{
    // determine which port is being connected:
    QString otherPort = ((QAction*)sender())->data().toString();
    client->connectPorts(fullPortName, otherPort);
}

void GraphicsPortItem::onDisconnectAction()
{
    // determine which port is being disconnected:
    QString otherPort = ((QAction*)sender())->data().toString();
    client->disconnectPorts(fullPortName, otherPort);
}

GraphicsPortConnectionItem * GraphicsPortConnectionItem::getPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene)
{
    GraphicsPortConnectionItem *item = items.value(port1).value(port2, 0);
    if (!item) {
        item = new GraphicsPortConnectionItem(port1, port2, scene);
        items[port1][port2] = item;
        items[port2][port1] = item;
    }
    return item;
}

void GraphicsPortConnectionItem::deletePortConnectionItem(const QString &port1, const QString &port2)
{
    GraphicsPortConnectionItem *item = items.value(port1).value(port2, 0);
    if (item) {
        items[port1].remove(port2);
        items[port2].remove(port1);
        delete item;
    }
}

void GraphicsPortConnectionItem::setPos(const QString &port, const QPointF &point)
{
    if (port == port1) {
        point1 = point;
    } else if (port == port2) {
        point2 = point;
    }
    QPainterPath path(point1);
    path.lineTo(point2);
    setPath(path);
}

void GraphicsPortConnectionItem::setPositions(const QString &port, const QPointF &point)
{
    const QMap<QString, GraphicsPortConnectionItem*> &portItems = items[port];
    for (QMap<QString, GraphicsPortConnectionItem*>::const_iterator i = portItems.begin(); i != portItems.end(); i++) {
        i.value()->setPos(port, point);
    }
}

GraphicsPortConnectionItem::GraphicsPortConnectionItem(const QString &port1_, const QString &port2_, QGraphicsScene *scene) :
    QGraphicsPathItem(0, scene),
    port1(port1_),
    port2(port2_)
{
}

QMap<QString, QMap<QString, GraphicsPortConnectionItem*> > GraphicsPortConnectionItem::items;
