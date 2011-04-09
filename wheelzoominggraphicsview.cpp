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

#include "wheelzoominggraphicsview.h"
#include <QMouseEvent>

WheelZoomingGraphicsView::WheelZoomingGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void WheelZoomingGraphicsView::setScene(QGraphicsScene *scene)
{
    QGraphicsView::setScene(scene);
    sceneChanged();
}

void WheelZoomingGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0) {
        scale(1.25, 1.25);
    } else if (event->delta() < 0) {
        scale(0.8, 0.8);
    }
}

