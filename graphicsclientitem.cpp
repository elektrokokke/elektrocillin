#include "graphicsclientitem.h"
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
    innerItem(0)
{
    setPen(QPen(QBrush(Qt::gray), 7, Qt::DashLine));
    setBrush(QBrush(QColor(Qt::gray).lighter()));
    QPointF offset = (rect.bottomRight() - rect.topLeft()) * 0.25 * (2 - sqrt(2.0));
    innerRect = QRectF(rect.topLeft() + offset, rect.bottomRight() - offset);
    // show the client name on the bottom:
    {
        QLabel *label = new QLabel(client->getClientName());
        label->setFrameStyle(QFrame::Raised | QFrame::Panel);
        QGraphicsProxyWidget *labelProxy = new QGraphicsProxyWidget(this);
        labelProxy->setWidget(label);
        labelProxy->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        labelProxy->setPos(innerRect.bottomLeft());
        labelProxy->setZValue(1);
    }
    {
        // show the client's inputs on the left:
        QWidget *widget = new QWidget();
        widget->setLayout(new QVBoxLayout());
        widget->layout()->setContentsMargins(0, 0, 0, 0);
        widget->layout()->setSpacing(0);
        QStringList portNames = client->getClientPorts(client->getClientName(), JackPortIsInput);
        for (int i = 0; i < portNames.size(); i++) {
            QLabel *label = new QLabel(portNames[i].split(':')[1]);
            label->setFrameStyle(QFrame::Raised | QFrame::Panel);
            widget->layout()->addWidget(label);
        }
        widget->adjustSize();
        QGraphicsRectItem *proxyRect = new QGraphicsRectItem(QRectF(0, 0, widget->width(), widget->height()), this);
        proxyRect->setPen(QPen(Qt::NoPen));
        proxyRect->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        proxyRect->setPos(innerRect.left(), 0.5 * (innerRect.top() + innerRect.bottom()));
        QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(proxyRect);
        proxy->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        proxy->setWidget(widget);
        proxy->setPos(proxyRect->rect().right(), 0.5 * (proxyRect->rect().top() + proxyRect->rect().bottom()));
        proxy->rotate(180);
        proxyRect->rotate(180);
    }
    {
        // show the client's outputs on the left:
        QWidget *widget = new QWidget();
        widget->setLayout(new QVBoxLayout());
        widget->layout()->setContentsMargins(0, 0, 0, 0);
        widget->layout()->setSpacing(0);
        QStringList portNames = client->getClientPorts(client->getClientName(), JackPortIsOutput);
        for (int i = 0; i < portNames.size(); i++) {
            QLabel *label = new QLabel(portNames[i].split(':')[1]);
            label->setFrameStyle(QFrame::Raised | QFrame::Panel);
            widget->layout()->addWidget(label);
        }
        widget->adjustSize();
        QGraphicsRectItem *proxyRect = new QGraphicsRectItem(QRectF(0, 0, widget->width(), widget->height()), this);
        proxyRect->setPen(QPen(Qt::NoPen));
        proxyRect->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        proxyRect->setPos(innerRect.right(), 0.5 * (innerRect.top() + innerRect.bottom()));
        QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(proxyRect);
        proxy->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        proxy->setWidget(widget);
        proxy->setPos(proxyRect->rect().left(), -0.5 * (proxyRect->rect().top() + proxyRect->rect().bottom()));
    }
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
