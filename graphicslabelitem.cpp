#include "graphicslabelitem.h"
#include <QBrush>

GraphicsLabelItem::GraphicsLabelItem(QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    padding(2)
{
    setBrush(QBrush(QColor(0xfc, 0xf9, 0xc2)));
    textItem = new QGraphicsSimpleTextItem(this);
    textItem->setPos(padding, padding);
}

GraphicsLabelItem::GraphicsLabelItem(const QString &text, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    padding(2)
{
    setBrush(QBrush(QColor(0xfc, 0xf9, 0xc2)));
    textItem = new QGraphicsSimpleTextItem(this);
    textItem->setPos(padding, padding);
    setText(text);
}

void GraphicsLabelItem::setText(const QString &text)
{
    textItem->setText(text);
    setRect(textItem->boundingRect().adjusted(0, 0, padding * 2, padding * 2));
}

void GraphicsLabelItem::setFont(const QFont &font)
{
    textItem->setFont(font);
    setRect(textItem->boundingRect().adjusted(0, 0, padding * 2, padding * 2));
}
