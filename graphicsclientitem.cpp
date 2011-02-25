#include "graphicsclientitem.h"
#include "graphicsclientnameitem.h"
#include "graphicsportitem.h"
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsProxyWidget>
#include <QPen>
#include <cmath>

GraphicsClientItem::GraphicsClientItem(JackClient *client_, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    client(client_),
    clientName(client->getClientName()),
    innerItem(0)
{
    init();
}

GraphicsClientItem::GraphicsClientItem(JackClient *client_, const QString &clientName_, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    client(client_),
    clientName(clientName_),
    innerItem(0)
{
    init();
}

void GraphicsClientItem::init()
{
    padding = 1;
    fill = QBrush(Qt::white);
    outline = QPen(QBrush(Qt::black), 1, Qt::DotLine);
    setBrush(fill);
    setPen(outline);


    QRectF boundingRectangle;
    qreal x;
    qreal y = 0;
    qreal minimumWidth = 0;
    QString portNamePattern = clientName + ":.*";
    // create input ports:
    x = 0;
    QStringList inputPorts = client->getPorts(portNamePattern.toAscii().data(), 0, JackPortIsInput);
    QList<GraphicsPortItem*> inputPortItems;
    for (int i = 0; i < inputPorts.size(); i++) {
        GraphicsPortItem *portItem = new GraphicsPortItem(client, inputPorts[i], this);
        inputPortItems.append(portItem);
        inputPortItems[i]->setPos(x, y);
        x += portItem->boundingRect().width() + padding;
        boundingRectangle |= portItem->boundingRect().translated(portItem->pos());
    }
    if (minimumWidth < x) {
        minimumWidth = x;
    }
    if (inputPortItems.size()) {
        y += inputPortItems.first()->boundingRect().height() + padding;
    }
    // create client name:
    x = 0;
    GraphicsClientNameItem *clientNameItem = new GraphicsClientNameItem(clientName, this);
    clientNameItem->setPos(x, y);
    x += clientNameItem->boundingRect().width() + padding;
    if (minimumWidth < x) {
        minimumWidth = x;
    }
    y += clientNameItem->boundingRect().height() + padding;
    // create output ports:
    x = 0;
    QStringList outputPorts = client->getPorts(portNamePattern.toAscii().data(), 0, JackPortIsOutput);
    QList<GraphicsPortItem*> outputPortItems;
    for (int i = 0; i < outputPorts.size(); i++) {
        GraphicsPortItem *portItem = new GraphicsPortItem(client, outputPorts[i], this);
        outputPortItems.append(portItem);
        outputPortItems[i]->setPos(x, y);
        x += portItem->boundingRect().width() + padding;
        boundingRectangle |= portItem->boundingRect().translated(portItem->pos());
    }
    if (minimumWidth < x) {
        minimumWidth = x;
    }
    if (outputPortItems.size()) {
        y += outputPortItems.first()->boundingRect().height() + padding;
    }
    clientNameItem->setMinimumWidth(minimumWidth - padding);
    boundingRectangle |= clientNameItem->boundingRect().translated(clientNameItem->pos());
    setRect(boundingRectangle.adjusted(-padding, -padding, padding, padding));
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemSendsScenePositionChanges);
}

JackClient * GraphicsClientItem::getClient()
{
    return client;
}

const QRectF & GraphicsClientItem::getInnerRect() const
{
    return innerRect;
}

QGraphicsItem * GraphicsClientItem::getInnerItem()
{
    return innerItem;
}

void GraphicsClientItem::setInnerItem(QGraphicsItem *item)
{
    QRectF boundingRectangle = rect().adjusted(padding, padding, -padding, -padding);
    innerRect = QRectF(boundingRectangle.right(), 0, 100, boundingRectangle.height());
    boundingRectangle |= innerRect;
    setRect(boundingRectangle.adjusted(-padding, -padding, padding, padding));

    innerItem = item;
    // add the item to our children:
    item->setParentItem(this);
    fitItemIntoRectangle(item, innerRect);
}

void GraphicsClientItem::fitItemIntoRectangle(QGraphicsItem *item, const QRectF &rect)
{
    QRectF boundingRect = item->boundingRect();
    // scale the item such that it fits into our rectangle:
    qreal scale = qMin(rect.width() / boundingRect.width(), rect.height() / boundingRect.height());
    item->setScale(scale);
    QPointF oldTopLeftScaled(boundingRect.topLeft() * scale);
    QPointF newTopLeftScaled(rect.left() + (rect.width() - boundingRect.width() * scale) * 0.5, rect.top() + (rect.height() - boundingRect.height() * scale) * 0.5);
    item->setPos(item->pos() - oldTopLeftScaled + newTopLeftScaled);
}
