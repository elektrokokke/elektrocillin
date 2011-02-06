#include "visiblerectanglegraphicsview.h"

VisibleRectangleGraphicsView::VisibleRectangleGraphicsView(QWidget *parent) :
    QGraphicsView(parent),
    animation(this, "visibleSceneRect")
{
}

VisibleRectangleGraphicsView::VisibleRectangleGraphicsView(QGraphicsScene *scene, QWidget *parent) :
    QGraphicsView(scene, parent)
{
}

QRectF VisibleRectangleGraphicsView::visibleSceneRect() const
{
    // compute visible rectangle from viewport rectangel and the current transform:
    QRect rectViewport = viewport()->rect();
    return mapToScene(rectViewport).boundingRect();
}

void VisibleRectangleGraphicsView::setVisibleSceneRect(const QRectF &rect)
{
    fitInView(rect, Qt::KeepAspectRatio);
}

void VisibleRectangleGraphicsView::animateToVisibleSceneRect(const QRectF &rect, int msecs)
{
    animation.stop();
    animation.setDuration(msecs);
    animation.setStartValue(visibleSceneRect());
    animation.setKeyValueAt(0.5, sceneRect());
    animation.setEndValue(rect);
    animation.start();
}
