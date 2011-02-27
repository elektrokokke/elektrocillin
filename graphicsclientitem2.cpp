#include "graphicsclientitem2.h"
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <QBrush>

GraphicsClientItem2::GraphicsClientItem2(const QString &clientName, QGraphicsItem *parent) :
    QGraphicsPathItem(parent)
{
    QColor captionColor(0xfc, 0xf9, 0xc2);
    QFont font("Mighty Zeo 2.0", 16);
    font.setStyleStrategy(QFont::PreferAntialias);
    QFontMetrics fontMetrics(font);
    int padding = fontMetrics.height() * 2;
    int portPadding = fontMetrics.height() / 2;

    QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem(clientName, this);
    textItem->setFont(font);
    textItem->setPos(padding, padding);

    QPainterPath path;
    path.addEllipse(textItem->boundingRect().adjusted(0, 0, padding*2, padding*2));

    QStringList inputPorts;
    inputPorts.append("Audio in");
    inputPorts.append("Midi in");
    inputPorts.append("Cutoff modulation");
    for (int i = 0, x = 0; i < inputPorts.size(); i++) {
        QGraphicsSimpleTextItem *portTextItem = new QGraphicsSimpleTextItem(inputPorts[i], this);
        portTextItem->setFont(font);
        portTextItem->setPos(x, 0);
        portTextItem->setFlags(QGraphicsItem::ItemStacksBehindParent);
        portTextItem->setZValue(1);
        QRectF portRect(portTextItem->boundingRect().adjusted(-portPadding, -portPadding, portPadding, portPadding).translated(portTextItem->pos()));
        QGraphicsRectItem *captionItem = new QGraphicsRectItem(portRect, this);
        captionItem->setPen(QPen(QBrush(Qt::black), 2));
        captionItem->setBrush(QBrush(captionColor));
        captionItem->setFlags(QGraphicsItem::ItemStacksBehindParent);
        if (i == 0) {
            captionItem = new QGraphicsRectItem(portRect.adjusted(4, 4, -4, -4), this);
            captionItem->setPen(QPen(QBrush(Qt::black), 2));
            captionItem->setFlags(QGraphicsItem::ItemStacksBehindParent);
        }
        QPainterPath portPath(0.5 * (portRect.topLeft() + portRect.bottomLeft()));
        portPath.lineTo(portRect.topLeft());
        portPath.lineTo(portRect.topRight());
        portPath.lineTo(0.5 * (portRect.topRight() + portRect.bottomRight()));
        portPath.arcTo(portRect, 0, -180);
        path = path.subtracted(portPath);
        x += portRect.width() + portPadding;
    }
    QStringList outputPorts;
    outputPorts.append("Audio out");
    for (int i = 0, x = 0; i < outputPorts.size(); i++) {
        QGraphicsSimpleTextItem *portTextItem = new QGraphicsSimpleTextItem(outputPorts[i], this);
        portTextItem->setFont(font);
        portTextItem->setPos(x, padding * 2);
        portTextItem->setFlags(QGraphicsItem::ItemStacksBehindParent);
        portTextItem->setZValue(1);
        QRectF portRect(portTextItem->boundingRect().adjusted(-portPadding, -portPadding, portPadding, portPadding).translated(portTextItem->pos()));
        QGraphicsRectItem *captionItem = new QGraphicsRectItem(portRect, this);
        captionItem->setPen(QPen(QBrush(Qt::black), 2));
        captionItem->setBrush(QBrush(captionColor));
        captionItem->setFlags(QGraphicsItem::ItemStacksBehindParent);
        QPainterPath portPath(0.5 * (portRect.topLeft() + portRect.bottomLeft()));
        portPath.lineTo(portRect.bottomLeft());
        portPath.lineTo(portRect.bottomRight());
        portPath.lineTo(0.5 * (portRect.topRight() + portRect.bottomRight()));
        portPath.arcTo(portRect, 0, 180);
        path = path.subtracted(portPath);
        x += portRect.width() + portPadding;
    }

    setPath(path);
    setPen(QPen(QBrush(Qt::black), 3));
    setBrush(QBrush(Qt::white));
    setFlags(QGraphicsItem::ItemIsMovable);
}
