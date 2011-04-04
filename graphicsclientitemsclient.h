#ifndef GRAPHICSCLIENTITEMSCLIENT_H
#define GRAPHICSCLIENTITEMSCLIENT_H

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

#include "jackclient.h"
#include "graphicsclientitem.h"
#include "graphicsportconnectionitem.h"
#include <QGraphicsScene>
#include <QMap>
#include <QSettings>

class GraphicsClientItemsClient : public JackClient
{
    Q_OBJECT
public:
    GraphicsClientItemsClient(QGraphicsScene *scene);
    virtual ~GraphicsClientItemsClient();

    void saveState(QDataStream &stream);
    void loadState(QDataStream &stream);

    void setClientStyle(int clientStyle);
    void setAudioPortStyle(int audioPortStyle);
    void setMidiPortStyle(int midiPortStyle);

    void deleteClient(const QString &clientName);

    QGraphicsScene * getScene();

    void showAllInnerItems(bool visible = true);

    GraphicsPortConnectionItem * getPortConnectionItem(const QString &port1, const QString &port2);
    void deletePortConnectionItem(QString port1, QString port2);
    void deletePortConnectionItems(QString port);
    void setPositions(const QString &port, const  QPointF &point);

    void setClientItemPositionByName(const QString &clientName, QPointF pos);

    static QSettings * getSettings();
public slots:
    void onClientRegistered(const QString &clientName);
    void onClientUnregistered(const QString &clientName);
    void onPortRegistered(QString fullPortName, QString type, int flags);
private:
    QGraphicsScene *scene;
    QMap<QString, GraphicsClientItem*> clientItems;
    QMap<QString, QPointF> clientItemPositionMap;
    QMap<QString, QMap<QString, GraphicsPortConnectionItem*> > portConnectionItems;
    int clientStyle, audioPortStyle, midiPortStyle;
    QFont font;
    QString contextName;

    static QSettings settings;
};

#endif // GRAPHICSCLIENTITEMSCLIENT_H
