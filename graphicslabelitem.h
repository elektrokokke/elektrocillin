#ifndef GRAPHICSLABELITEM_H
#define GRAPHICSLABELITEM_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>

class GraphicsLabelItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    GraphicsLabelItem(QGraphicsItem *parent = 0);
    GraphicsLabelItem(const QString &text, QGraphicsItem *parent = 0);
    void setFont(const QFont &font);
public slots:
    void setText(QString text);
private:
    QGraphicsSimpleTextItem *textItem;
    int padding;
};

#endif // GRAPHICSLABELITEM_H
