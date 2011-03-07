#include "wheelzoominggraphicsview.h"
#include <QMouseEvent>

WheelZoomingGraphicsView::WheelZoomingGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
}

void WheelZoomingGraphicsView::setScene(QGraphicsScene *scene)
{
    QGraphicsView::setScene(scene);
    sceneChanged();
}

void WheelZoomingGraphicsView::wheelEvent(QWheelEvent *event)
{
    // get the scene position of the mouse position:
    QPointF scenePosUnderMouseBefore = mapToScene(event->pos());
    if (event->delta() > 0) {
        scale(1.25, 1.25);
    } else if (event->delta() < 0) {
        scale(0.8, 0.8);
    }
    QPointF scenePosUnderMouseAfter = mapToScene(event->pos());
    QPointF scenePosUnderCenterAfter = mapToScene(QPoint(viewport()->rect().width() / 2, viewport()->rect().height() / 2));
    centerOn(scenePosUnderCenterAfter + scenePosUnderMouseBefore - scenePosUnderMouseAfter);
}

