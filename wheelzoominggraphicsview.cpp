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

