#include "graphicsclientitem.h"
#include "graphicsportitem.h"
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsProxyWidget>
#include <QPen>
#include <cmath>

GraphicsClientItem::GraphicsClientItem(JackClient *client_, const QRectF &rect, QGraphicsItem *parent) :
    QGraphicsEllipseItem(rect, parent),
    client(client_),
    clientName(client->getClientName()),
    innerItem(0)
{
    init();
}

GraphicsClientItem::GraphicsClientItem(JackClient *client_, const QString &clientName_, const QRectF &rect, QGraphicsItem *parent) :
    QGraphicsEllipseItem(rect, parent),
    client(client_),
    clientName(clientName_),
    innerItem(0)
{
    init();
}

void GraphicsClientItem::init()
{
    setPen(QPen(QBrush(Qt::lightGray), 10, Qt::DashLine));
    setBrush(QBrush(Qt::lightGray));
    QPointF offset = (rect().bottomRight() - rect().topLeft()) * 0.25 * (2 - sqrt(2.0));
    innerRect = QRectF(rect().topLeft() + offset, rect().bottomRight() - offset);

    QString portNamePattern = clientName + ":.*";
    QStringList inputPorts = client->getPorts(portNamePattern.toAscii().data(), 0, JackPortIsInput);
    QList<GraphicsPortItem*> inputPortItems;
    qreal inputsHeight = 0;
    for (int i = 0; i < inputPorts.size(); i++) {
        GraphicsPortItem *portItem = new GraphicsPortItem(client, inputPorts[i], this);
        inputsHeight += portItem->rect().height();
        inputPortItems.append(portItem);
    }
    for (int i = 0; i < inputPorts.size(); i++) {
        inputPortItems[i]->setPos(innerRect.left() - inputPortItems[i]->rect().width(), rect().top() + 0.5 * (rect().height() - inputsHeight) + inputsHeight * i / inputPortItems.size());
    }
    QStringList outputPorts = client->getPorts(portNamePattern.toAscii().data(), 0, JackPortIsOutput);
    QList<GraphicsPortItem*> outputPortItems;
    qreal outputsHeight = 0;
    for (int i = 0; i < outputPorts.size(); i++) {
        GraphicsPortItem *portItem = new GraphicsPortItem(client, outputPorts[i], this);
        outputsHeight += portItem->rect().height();
        outputPortItems.append(portItem);
    }
    for (int i = 0; i < outputPorts.size(); i++) {
        outputPortItems[i]->setPos(innerRect.right(), rect().top() + 0.5 * (rect().height() - outputsHeight) + outputsHeight * i / outputPortItems.size());
    }
    setFlags(QGraphicsItem::ItemIsMovable);
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
    innerItem = item;
    // add the item to our children:
    item->setParentItem(this);
    fitItemIntoRectangle(item, getInnerRect());
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
