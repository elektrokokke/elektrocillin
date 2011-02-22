#include "graphicsportitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QBrush>
#include <QFont>
#include <QSet>

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
    QString otherPort = ((QAction*)sender())->data().toString();
    client->connectPorts(fullPortName, otherPort);
}

void GraphicsPortItem::onDisconnectAction()
{
    // determine which port is being disconnected:
    QString otherPort = ((QAction*)sender())->data().toString();
    client->disconnectPorts(fullPortName, otherPort);
}
