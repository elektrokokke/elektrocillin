#include "graphicsportitem2.h"
#include "graphicsclientitem2.h"
#include "graphicsportconnectionitem.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QFontMetrics>
#include <QCursor>

GraphicsPortItem2::GraphicsPortItem2(JackClient *client_, const QString &fullPortName_, int style_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    client(client_),
    fullPortName(fullPortName_),
    shortPortName(fullPortName.split(":")[1]),
    dataType(client->getPortType(fullPortName)),
    isInput(client->getPortFlags(fullPortName) & JackPortIsInput),
    style(style_)
{
    QColor captionColor(0xfc, 0xf9, 0xc2);
    setPen(QPen(QBrush(Qt::black), 2));
    setBrush(QBrush(captionColor));
    setFlags(QGraphicsItem::ItemSendsScenePositionChanges);
    setCursor(Qt::ArrowCursor);
    QFont font("Mighty Zeo 2.0", 12);
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

    QLinearGradient gradient(portRect.topLeft(), portRect.bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, QColor("wheat"));
    setBrush(QBrush(gradient));

}

const QRectF & GraphicsPortItem2::getRect() const
{
    return portRect;
}

void GraphicsPortItem2::connectedTo(const QString &otherPort)
{
//    connectMenu->removeAction(mapPortNamesToActions[otherPort]);
//    QAction *action = disconnectMenu->addAction(otherPort);
//    action->setData(otherPort);
//    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onDisconnectAction()));
//    mapPortNamesToActions[otherPort] = action;
//    disconnectMenu->setEnabled(disconnectMenu->actions().size());
//    connectMenu->setEnabled(connectMenu->actions().size());
    GraphicsPortConnectionItem *connectionItem = GraphicsPortConnectionItem::getPortConnectionItem(fullPortName, otherPort, scene());
    connectionItem->setPos(fullPortName, sceneBoundingRect().center());
//    connections++;
}

void GraphicsPortItem2::disconnectedFrom(const QString &otherPort)
{
//    disconnectMenu->removeAction(mapPortNamesToActions[otherPort]);
//    QAction *action = connectMenu->addAction(otherPort);
//    action->setData(otherPort);
//    QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
//    mapPortNamesToActions[otherPort] = action;
//    disconnectMenu->setEnabled(disconnectMenu->actions().size());
//    connectMenu->setEnabled(connectMenu->actions().size());
    GraphicsPortConnectionItem::deletePortConnectionItem(fullPortName, otherPort);
//    connections--;
}

void GraphicsPortItem2::registeredPort(const QString &fullPortname, const QString &type, int flags)
{
//    bool portIsInput = (flags & JackPortIsInput);
//    if ((this->type == type) && (portIsInput != isInput)) {
//        // put the port into our connection menu:
//        QAction *action = connectMenu->addAction(fullPortname);
//        action->setData(fullPortname);
//        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onConnectAction()));
//        mapPortNamesToActions[fullPortname] = action;
//        connectMenu->setEnabled(connectMenu->actions().size());
//    }
}

void GraphicsPortItem2::unregisteredPort(const QString &fullPortname, const QString &type, int flags)
{
//    QAction *action = mapPortNamesToActions.value(fullPortname, 0);
//    if (action) {
//        // remove the port from our menus:
//        connectMenu->removeAction(action);
//        disconnectMenu->removeAction(action);
//        mapPortNamesToActions.remove(fullPortName);
//        disconnectMenu->setEnabled(disconnectMenu->actions().size());
//        connectMenu->setEnabled(connectMenu->actions().size());
//    }
}

QVariant GraphicsPortItem2::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged) {
        GraphicsPortConnectionItem::setPositions(fullPortName, sceneBoundingRect().center());
    }
    return QGraphicsItem::itemChange(change, value);
}
