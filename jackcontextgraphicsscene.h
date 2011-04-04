#ifndef JACKCONTEXTGRAPHICSSCENE_H
#define JACKCONTEXTGRAPHICSSCENE_H

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

#include "graphicsclientitem.h"
#include "graphicsclientitemsclient.h"
#include "jacknullclient.h"
#include <QGraphicsScene>
#include <QFont>

class JackContext;

class JackContextGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    JackContextGraphicsScene();
    virtual ~JackContextGraphicsScene();

    void saveSession(QDataStream &stream);
    void loadSession(QDataStream &stream);
    void loadMacro(const QString &fileName);

    void changeToCurrentContext();
    void deleteClient(const QString &clientName);

    bool editMacro(GraphicsClientItem *macroItem);
signals:
    void contextLevelChanged(int level);
    void messageChanged(QString message);
public slots:
    void showAllInnerItems(bool visible = true);
    void play();
    void stop();
    void rewind();
    void exitCurrentMacro();
    void editSelectedMacro();
    void createNewMacro();
    void createNewModule(QString factoryName);
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent);
private:
    GraphicsClientItemsClient *graphicsClientItemsClient;
    bool waitForMacroPosition, waitForModulePosition;
    QString factoryName, macroFileName;

    void createNewMacro(QPointF pos);
    void createNewModule(QString factoryName, QPointF pos);
};

#endif // JACKCONTEXTGRAPHICSSCENE_H
