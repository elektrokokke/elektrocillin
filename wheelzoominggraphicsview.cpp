#include "wheelzoominggraphicsview.h"
#include <QMouseEvent>

WheelZoomingGraphicsView::WheelZoomingGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
}

void WheelZoomingGraphicsView::wheelEvent(QWheelEvent *event)
{
    // get the scene position of the mouse position:
    QPointF pos = mapToScene(event->pos());
    if (event->delta() > 0) {
        scale(1.25, 1.25);
    } else if (event->delta() < 0) {
        scale(0.8, 0.8);
    }
    centerOn(pos);
}

