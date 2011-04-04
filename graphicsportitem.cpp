/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphicsportitem.h"
#include "graphicsclientitem.h"
#include "graphicsportconnectionitem.h"
#include "graphicsclientitemsclient.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QFontMetrics>
#include <QCursor>
#include <QSet>
#include <QGraphicsSceneMouseEvent>

GraphicsPortItem::GraphicsPortItem(GraphicsClientItemsClient *client_, const QString &fullPortName_, int style_, QFont font_, int padding, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    client(client_),
    fullPortName(fullPortName_),
    shortPortName(fullPortName.split(":")[1]),
    dataType(client->getPortType(fullPortName)),
    isInput(client->getPortFlags(fullPortName) & JackPortIsInput),
    style(style_),
    font(font_),
    showMenu(false)
{
    bool gradient = false;
    QColor captionColor(0xfc, 0xf9, 0xc2);
    setPen(QPen(QBrush(Qt::black), 2));
    setBrush(QBrush(captionColor));
    setFlags(QGraphicsItem::ItemSendsScenePositionChanges);
    setCursor(Qt::ArrowCursor);
    font.setStyleStrategy(QFont::PreferAntialias);
    QFontMetrics fontMetrics(font);
    int portPadding = padding;

    QGraphicsSimpleTextItem *portTextItem = new QGraphicsSimpleTextItem(shortPortName, this);
    portTextItem->setFont(font);
    portTextItem->setPos(portPadding, 0);
    portRect = portTextItem->boundingRect().adjusted(-portPadding, -portPadding, portPadding, portPadding).translated(portTextItem->pos());

    QPainterPath portPath;
    if (style == 0) {
        portPath = portPath.united(EllipsePath(portRect));
    } else if (style == 1) {
        portPath = portPath.united(SpeechBubblePath(portRect, portRect.height() / 4, portRect.height() / 4, Qt::AbsoluteSize));
    } else if (style == 2) {
        portPath = portPath.united(RoundedRectanglePath(portRect, portPadding + fontMetrics.height() / 2, portPadding + fontMetrics.height() / 2));
    } else if (style == 3) {
        portPath = portPath.united(RectanglePath(portRect));
    }
    setPath(portPath);

    // register the port registration callback at the jack server:
    QObject::connect(client, SIGNAL(portRegistered(QString,QString,int)), this, SLOT(onPortRegistered(QString,QString,int)), Qt::QueuedConnection);
    QObject::connect(client, SIGNAL(portUnregistered(QString,QString,int)), this, SLOT(onPortUnregistered(QString,QString,int)), Qt::QueuedConnection);
    QObject::connect(client, SIGNAL(portConnected(QString,QString)), this, SLOT(onPortConnected(QString,QString)), Qt::QueuedConnection);
    QObject::connect(client, SIGNAL(portDisconnected(QString,QString)), this, SLOT(onPortDisconnected(QString,QString)), Qt::QueuedConnection);

    if (gradient) {
        QLinearGradient gradient(portRect.topLeft(), portRect.bottomRight());
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1, QColor("wheat"));
        setBrush(QBrush(gradient));
    }

    // create the context menu:
    connectMenu = contextMenu.addMenu("Connect");
    disconnectMenu = contextMenu.addMenu("Disconnect");
    // create the entries in connect- and disconnect-menus, as well as graphical representations of existing connections:
    QStringList connectedPorts = client->getConnectedPorts(fullPortName);
    QSet<QString> connectedPortsSet;
    for (int i = 0; i < connectedPorts.size(); i++) {
        // create an entry in the disconnect-menu:
        QAction *action = disconnectMenu->addAction(connectedPorts[i]);
        action->setData(connectedPorts[i]);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onDisconnectAction()));
        mapPortNamesToActions[connectedPorts[i]] = action;
        connectedPortsSet.insert(connectedPorts[i]);
        // create a graphical representation of the connection:
        if (isInput) {
            client->getPortConnectionItem(connectedPorts[i], fullPortName)->setPos(fullPortName, getConnectionScenePos());
        } else {
            client->getPortConnectionItem(fullPortName, connectedPorts[i])->setPos(fullPortName, getConnectionScenePos());
        }
    }
    // get all available ports that can be connected to this:
    QStringList connectablePorts = client->getPorts(0, dataType.toAscii().data(), isInput ? JackPortIsOutput : JackPortIsInput);
    for (int i = 0; i < connectablePorts.size(); i++) {
        // skip ports that are already connected:
        if (!connectedPortsSet.contains(connectablePorts[i])) {
            // create an entry in the connect-menu:
            QAction *action = connectMenu->addAction(connectablePorts[i]);
            action->setData(connectablePorts[i]);
            QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
            mapPortNamesToActions[connectablePorts[i]] = action;
        }
    }
    disconnectMenu->setEnabled(disconnectMenu->actions().size());
    connectMenu->setEnabled(connectMenu->actions().size());
}

const QRectF & GraphicsPortItem::getRect() const
{
    return portRect;
}

QPointF GraphicsPortItem::getConnectionScenePos() const
{
    QRectF sceneRect = sceneBoundingRect();
    return (isInput ? QPointF(sceneRect.center().x(), sceneRect.top()) : QPointF(sceneRect.center().x(), sceneRect.bottom()));
}

const QString & GraphicsPortItem::getDataType() const
{
    return dataType;
}

bool GraphicsPortItem::isAudioType() const
{
    return dataType == JACK_DEFAULT_AUDIO_TYPE;
}

void GraphicsPortItem::onPortRegistered(QString fullPortName, QString type, int flags)
{
    bool portIsInput = (flags & JackPortIsInput);
    if ((this->dataType == type) && (portIsInput != isInput)) {
        // put the port into our connection menu:
        QAction *action = connectMenu->addAction(fullPortName);
        action->setData(fullPortName);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
        mapPortNamesToActions[fullPortName] = action;
        connectMenu->setEnabled(connectMenu->actions().size());
    }
}

void GraphicsPortItem::onPortUnregistered(QString fullPortName, QString, int)
{
    QAction *action = mapPortNamesToActions.value(fullPortName, 0);
    if (action) {
        // remove the port from our menus:
        connectMenu->removeAction(action);
        disconnectMenu->removeAction(action);
        mapPortNamesToActions.remove(fullPortName);
        disconnectMenu->setEnabled(disconnectMenu->actions().size());
        connectMenu->setEnabled(connectMenu->actions().size());
    }
}

void GraphicsPortItem::onPortConnected(QString sourcePortName, QString destPortName)
{
    QAction *action = 0;
    if (isInput && (destPortName == fullPortName)) {
        client->getPortConnectionItem(sourcePortName, fullPortName)->setPos(fullPortName, getConnectionScenePos());
        action = mapPortNamesToActions.value(sourcePortName, 0);
    } else if (!isInput && (sourcePortName == fullPortName)){
        client->getPortConnectionItem(fullPortName, destPortName)->setPos(fullPortName, getConnectionScenePos());
        action = mapPortNamesToActions.value(destPortName, 0);
    }
    if (action) {
        QObject::disconnect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onDisconnectAction()));
        connectMenu->removeAction(action);
        disconnectMenu->addAction(action);
        disconnectMenu->setEnabled(disconnectMenu->actions().size());
        connectMenu->setEnabled(connectMenu->actions().size());
    }
}

void GraphicsPortItem::onPortDisconnected(QString sourcePortName, QString destPortName)
{
    QAction *action = 0;
    if (isInput && (destPortName == fullPortName)) {
        client->deletePortConnectionItem(sourcePortName, fullPortName);
        action = mapPortNamesToActions.value(sourcePortName, 0);
    } else if (!isInput && (sourcePortName == fullPortName)){
        client->deletePortConnectionItem(fullPortName, destPortName);
        action = mapPortNamesToActions.value(destPortName, 0);
    }
    if (action) {
        QObject::disconnect(action, SIGNAL(triggered()), this, SLOT(onDisconnectAction()));
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
        disconnectMenu->removeAction(action);
        connectMenu->addAction(action);
        disconnectMenu->setEnabled(disconnectMenu->actions().size());
        connectMenu->setEnabled(connectMenu->actions().size());
    }
}

void GraphicsPortItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsPathItem::mousePressEvent(event);
    if (!event->isAccepted()) {
        event->accept();
        showMenu = true;
    }
}

void GraphicsPortItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    if (showMenu) {
        showMenu = false;
        // show a menu that allows removing and adding control points:
        contextMenu.popup(event->screenPos());
    } else {
        QGraphicsPathItem::mouseReleaseEvent(event);
    }
}

QVariant GraphicsPortItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged) {
        client->setPositions(fullPortName, getConnectionScenePos());
    }
    return QGraphicsItem::itemChange(change, value);
}

void GraphicsPortItem::onConnectAction()
{
    // determine which port is being connected:
    QString otherPort = ((QAction*)sender())->data().toString();
    if (isInput) {
        client->connectPorts(otherPort, fullPortName);
    } else {
        client->connectPorts(fullPortName, otherPort);
    }
}

void GraphicsPortItem::onDisconnectAction()
{
    // determine which port is being disconnected:
    QString otherPort = ((QAction*)sender())->data().toString();
    if (isInput) {
        client->disconnectPorts(otherPort, fullPortName);
    } else {
        client->disconnectPorts(fullPortName, otherPort);
    }
}
