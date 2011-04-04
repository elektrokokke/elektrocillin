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

#include "graphicslabelitem.h"
#include <QPen>
#include <QBrush>

GraphicsLabelItem::GraphicsLabelItem(QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    padding(2)
{
    setPen(QPen(QBrush(Qt::black), 1));
    setBrush(QBrush(QColor(0xfc, 0xf9, 0xc2)));
    textItem = new QGraphicsSimpleTextItem(this);
    textItem->setPos(padding, padding);
}

GraphicsLabelItem::GraphicsLabelItem(const QString &text, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    padding(2)
{
    setPen(QPen(QBrush(Qt::black), 1));
    setBrush(QBrush(QColor(0xfc, 0xf9, 0xc2)));
    textItem = new QGraphicsSimpleTextItem(this);
    textItem->setPos(padding, padding);
    setText(text);
}

void GraphicsLabelItem::setFont(const QFont &font)
{
    textItem->setFont(font);
    setRect(textItem->boundingRect().adjusted(0, 0, padding * 2, padding * 2));
}

void GraphicsLabelItem::setText(QString text)
{
    textItem->setText(text);
    QRectF rect = textItem->boundingRect();
    setRect(rect.adjusted(0, 0, padding * 2, padding * 2));
}

