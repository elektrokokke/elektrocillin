#include "graphicsportitem.h"
#include "graphicsportconnectionitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QSet>
#include <QGraphicsScene>
//#include <QGraphicsDropShadowEffect>

GraphicsPortItem::GraphicsPortItem(JackClient *client_, const QString &fullPortName_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    client(client_),
    clientName(client->getClientName()),
    fullPortName(fullPortName_),
    shortPortName(fullPortName.split(":")[1]),
    connections(0),
    gapSize(8),
    fill(QColor("wheat")),
    outline(QBrush(fill.color().darker()), 2),
    font("Mighty Zeo 2.0", 12)

{
    font.setStyleStrategy(QFont::PreferAntialias);
    setBrush(fill);
    setPen(outline);

    // get port info:
    type = client->getPortType(fullPortName);
    isInput = (client->getPortFlags(fullPortName) & JackPortIsInput);

    portNameItem = new QGraphicsSimpleTextItem(shortPortName, this);
    portNameItem->setFont(font);
    QRectF rect(portNameItem->boundingRect().adjusted(0, 0, 4, 4 + gapSize));
    QPainterPath path;
    if (isInput) {
        path.moveTo(rect.topLeft());
        path.lineTo(QPointF(rect.center().x() - gapSize, rect.top()));
        QRectF portConnectionPositionRect(path.currentPosition().x(), path.currentPosition().y() - gapSize, gapSize * 2, gapSize * 2);
        path.arcTo(portConnectionPositionRect, 180, 180);
        path.lineTo(rect.topRight());
        path.lineTo(rect.bottomRight());
        path.lineTo(rect.bottomLeft());
        path.lineTo(rect.topLeft());
        portConnectionPositionItem = new QGraphicsEllipseItem(portConnectionPositionRect, this);
        portConnectionPositionItem->setVisible(false);
    } else {
        path.moveTo(rect.bottomLeft());
        path.lineTo(QPointF(rect.center().x() - gapSize, rect.bottom()));
        QRectF portConnectionPositionRect(path.currentPosition().x(), path.currentPosition().y() - gapSize, gapSize * 2, gapSize * 2);
        path.arcTo(portConnectionPositionRect, 180, -180);
        path.lineTo(rect.bottomRight());
        path.lineTo(rect.topRight());
        path.lineTo(rect.topLeft());
        path.lineTo(rect.bottomLeft());
        portConnectionPositionItem = new QGraphicsEllipseItem(portConnectionPositionRect, this);
        portConnectionPositionItem->setVisible(false);
    }
    setPath(path);
    portConnectionPositionItem->setBrush(Qt::black);
    portConnectionPositionItem->setPen(QPen(Qt::NoPen));

    portNameItem->setPos(2, isInput ? 2 + gapSize : 2);
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
    setFlags(QGraphicsItem::ItemSendsScenePositionChanges);

//    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
//    effect->setBlurRadius(0);
//    setGraphicsEffect(effect);
}

GraphicsPortItem::~GraphicsPortItem()
{
    client->registerPortConnectInterface(fullPortName, 0);
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
    connectionItem->setPos(fullPortName, portConnectionPositionItem->sceneBoundingRect().center());
    connections++;
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
    connections--;
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
        GraphicsPortConnectionItem::setPositions(fullPortName, portConnectionPositionItem->sceneBoundingRect().center());
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
