#include "graphicsportitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QBrush>
#include <QFont>

GraphicsPortItem::GraphicsPortItem(JackClient *client_, const QString &fullPortName_, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    client(client_),
    fullPortName(fullPortName_),
    shortPortName(fullPortName.split(":")[1]),
    portNameItem(new QGraphicsSimpleTextItem(shortPortName, this))
{
    setBrush(QBrush(Qt::white));
    portNameItem->setFont(QFont("Helvetica", 8));
    setRect(portNameItem->boundingRect().adjusted(0, 0, 4, 2));
    portNameItem->setPos(2, 1);
    // create the context menu:
    connectMenu = contextMenu.addMenu("Connect");
    disconnectMenu = contextMenu.addMenu("Disconnect");
    QStringList connectedPorts = client->getConnectedPorts(fullPortName);
    for (int i = 0; i < connectedPorts.size(); i++) {
        QAction *action = disconnectMenu->addAction(connectedPorts[i]);
        mapPortNamesToActions[connectedPorts[i]] = action;
        mapActionsToPortNames[action] = connectedPorts[i];
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

void GraphicsPortItem::onConnectAction()
{
    // determine which port is being connected:
    QString otherPort = mapActionsToPortNames[(QAction*)sender()];
    client->connectPorts(fullPortName, otherPort);
}

void GraphicsPortItem::onDisconnectAction()
{
    // determine which port is being disconnected:
    QString otherPort = mapActionsToPortNames[(QAction*)sender()];
    client->disconnectPorts(fullPortName, otherPort);
}

void GraphicsPortItem::portConnected(const QString &otherPort)
{
    connectMenu->removeAction(mapPortNamesToActions[otherPort]);
    QAction *action = disconnectMenu->addAction(otherPort);
    mapPortNamesToActions[otherPort] = action;
    mapActionsToPortNames[action] = otherPort;
}

void GraphicsPortItem::portDisconnected(const QString &otherPort)
{
    disconnectMenu->removeAction(mapPortNamesToActions[otherPort]);
    QAction *action = connectMenu->addAction(otherPort);
    mapPortNamesToActions[otherPort] = action;
    mapActionsToPortNames[action] = otherPort;
}

void GraphicsPortItem::portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect)
{
    // get the full port names:
    QString aName = client->getPortNameById(a);
    QString bName = client->getPortNameById(b);
    if (aName == fullPortName) {
        if (connect) {
            portConnected(bName);
        } else {
            portDisconnected(bName);
        }
    } else if (bName == fullPortName) {
        if (connect) {
            portConnected(aName);
        } else {
            portDisconnected(aName);
        }
    }
}

void GraphicsPortItem::portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void* arg)
{
    GraphicsPortItem *portItem = (GraphicsPortItem*)arg;
    portItem->portConnectCallback(a, b, connect);
}
