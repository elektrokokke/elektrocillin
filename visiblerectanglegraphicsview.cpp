#include "visiblerectanglegraphicsview.h"
#include <QPropertyAnimation>

VisibleRectangleGraphicsView::VisibleRectangleGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

VisibleRectangleGraphicsView::VisibleRectangleGraphicsView(QGraphicsScene *scene, QWidget *parent) :
    QGraphicsView(scene, parent)
{
}

const QRectF & VisibleRectangleGraphicsView::visibleSceneRect() const
{
    return visibleSceneRectangle;
}

void VisibleRectangleGraphicsView::setVisibleSceneRect(const QRectF &rect)
{
    visibleSceneRectangle = rect;
    fitInView(rect, Qt::KeepAspectRatio);
}

void VisibleRectangleGraphicsView::animateToVisibleSceneRect(const QRectF &rect, int msecs)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "visibleSceneRect");
    animation->setDuration(msecs);
    animation->setStartValue(visibleSceneRect());
    animation->setKeyValueAt(0.5, sceneRect());
    animation->setEndValue(rect);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}
