#include "graphicsportitem.h"
#include "graphicsclientitem.h"
#include "graphicsportconnectionitem.h"
#include "jackcontextgraphicsscene.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QFontMetrics>
#include <QCursor>
#include <QSet>
#include <QGraphicsSceneMouseEvent>

GraphicsPortItem::GraphicsPortItem(JackClient *client_, const QString &fullPortName_, int style_, QFont font_, QGraphicsItem *parent, JackContextGraphicsScene *scene) :
    QGraphicsPathItem(parent, scene),
    client(client_),
    fullPortName(fullPortName_),
    shortPortName(fullPortName.split(":")[1]),
    dataType(client->getPortType(fullPortName)),
    isInput(client->getPortFlags(fullPortName) & JackPortIsInput),
    style(style_),
    font(font_),
    connections(0)
{
    bool gradient = false;
    QColor captionColor(0xfc, 0xf9, 0xc2);
    setPen(QPen(QBrush(Qt::black), 2));
    setBrush(QBrush(captionColor));
    setFlags(QGraphicsItem::ItemSendsScenePositionChanges);
    setCursor(Qt::ArrowCursor);
    font.setStyleStrategy(QFont::PreferAntialias);
    QFontMetrics fontMetrics(font);
    int portPadding = fontMetrics.height() / 2;

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

    // register the port connection callback at the jack server:
    client->registerPortConnectInterface(fullPortName, this);

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
        GraphicsPortConnectionItem *connectionItem = scene->getPortConnectionItem(fullPortName, connectedPorts[i], scene);
        connectionItem->setPos(fullPortName, getConnectionScenePos());
        connections++;
    }
    // get all available ports that can be connected to this:
    QStringList connectablePorts = client->getPorts(0, dataType.toAscii().data(), isInput ? JackPortIsOutput : JackPortIsInput);
    for (int i = 0; i < connectablePorts.size(); i++) {
        // skip ports that are already connected:
        if (!connectedPorts.contains(connectablePorts[i])) {
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

GraphicsPortItem::~GraphicsPortItem()
{
    // unregister the port connect callback:
    client->registerPortConnectInterface(fullPortName, 0);
}

const QRectF & GraphicsPortItem::getRect() const
{
    return portRect;
}

void GraphicsPortItem::connectedTo(const QString &otherPort)
{
    // remove the corresponding entry from the connect-menu:
    connectMenu->removeAction(mapPortNamesToActions[otherPort]);
    // create an entry in the disconnect-menu:
    QAction *action = disconnectMenu->addAction(otherPort);
    action->setData(otherPort);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onDisconnectAction()));
    mapPortNamesToActions[otherPort] = action;
    disconnectMenu->setEnabled(disconnectMenu->actions().size());
    connectMenu->setEnabled(connectMenu->actions().size());
    // create a graphical representation of the connection:
    GraphicsPortConnectionItem *connectionItem = ((JackContextGraphicsScene*)scene())->getPortConnectionItem(fullPortName, otherPort, scene());
    connectionItem->setPos(fullPortName, getConnectionScenePos());
    connections++;
}

void GraphicsPortItem::disconnectedFrom(const QString &otherPort)
{
    // remove the corresponding entry from the disconnect-menu:
    disconnectMenu->removeAction(mapPortNamesToActions[otherPort]);
    // create an entry in the connect-menu:
    QAction *action = connectMenu->addAction(otherPort);
    action->setData(otherPort);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
    mapPortNamesToActions[otherPort] = action;
    disconnectMenu->setEnabled(disconnectMenu->actions().size());
    connectMenu->setEnabled(connectMenu->actions().size());
    // delete the graphical representation of the connection:
    ((JackContextGraphicsScene*)scene())->deletePortConnectionItem(fullPortName, otherPort);
    connections--;
}

void GraphicsPortItem::registeredPort(const QString &fullPortname, const QString &type, int flags)
{
    bool portIsInput = (flags & JackPortIsInput);
    if ((this->dataType == type) && (portIsInput != isInput)) {
        // put the port into our connection menu:
        QAction *action = connectMenu->addAction(fullPortname);
        action->setData(fullPortname);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
        mapPortNamesToActions[fullPortname] = action;
        connectMenu->setEnabled(connectMenu->actions().size());
    }
}

void GraphicsPortItem::unregisteredPort(const QString &fullPortname, const QString &, int)
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

QPointF GraphicsPortItem::getConnectionScenePos() const
{
    QRectF sceneRect = sceneBoundingRect();
    return (isInput ? QPointF(sceneRect.center().x(), sceneRect.top()) : QPointF(sceneRect.center().x(), sceneRect.bottom()));
}

void GraphicsPortItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsPathItem::mousePressEvent(event);
    if (!event->isAccepted() && (event->button() == Qt::RightButton)) {
        event->accept();
        // show a menu that allows removing and adding control points:
        contextMenu.exec(event->screenPos());
    }
}

QVariant GraphicsPortItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged) {
        ((JackContextGraphicsScene*)scene())->setPositions(fullPortName, getConnectionScenePos());
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
