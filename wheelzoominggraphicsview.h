#ifndef VISIBLERECTANGLEGRAPHICSVIEW_H
#define VISIBLERECTANGLEGRAPHICSVIEW_H

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

#include <QGraphicsView>

class WheelZoomingGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit WheelZoomingGraphicsView(QWidget *parent = 0);

    void setScene(QGraphicsScene *scene);

signals:
    void sceneChanged();

protected:
    virtual void wheelEvent(QWheelEvent *event);
};

#endif // VISIBLERECTANGLEGRAPHICSVIEW_H
