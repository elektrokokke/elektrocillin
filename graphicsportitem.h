#ifndef GRAPHICSPORTITEM2_H
#define GRAPHICSPORTITEM2_H

/*
    Copyright 2011 Arne Jacobs

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

#include "jackclient.h"
#include <QGraphicsPathItem>
#include <QMenu>

class GraphicsClientItemsClient;

class GraphicsPortItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    GraphicsPortItem(GraphicsClientItemsClient *client, const QString &fullPortName, int style, QFont font, int padding, QGraphicsItem *parent);
    const QRectF & getRect() const;
    QPointF getConnectionScenePos() const;

    const QString & getDataType() const;
    bool isAudioType() const;

public slots:
    void onPortRegistered(QString fullPortName, QString style, int flags);
    void onPortUnregistered(QString fullPortName, QString style, int flags);
    void onPortConnected(QString sourcePortName, QString destPortName);
    void onPortDisconnected(QString sourcePortName, QString destPortName);

protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
private slots:
    void onConnectAction();
    void onDisconnectAction();
private:
    GraphicsClientItemsClient *client;
    QString fullPortName, shortPortName, dataType;
    bool isInput;
    int style;
    QFont font;
    QRectF portRect;
    QMenu contextMenu, *connectMenu, *disconnectMenu;
    QMap<QString, QAction*> mapPortNamesToActions;
    bool showMenu;
};

#endif // GRAPHICSPORTITEM2_H
