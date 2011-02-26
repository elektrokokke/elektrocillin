#include "visiblerectanglegraphicsview.h"
#include <QMouseEvent>

VisibleRectangleGraphicsView::VisibleRectangleGraphicsView(QWidget *parent) :
    QGraphicsView(parent),
    animation(this, "visibleSceneRect"),
    moving(false)
{
    QObject::connect(&animation, SIGNAL(finished()), this, SLOT(animationFinished()));
    setDragMode(QGraphicsView::ScrollHandDrag);
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
    QRect from = visibleSceneRect().toRect();
    QRect to = rect.toRect();
    if ((qreal)from.width() / (qreal)from.height() > (qreal)to.width() / (qreal)to.height()) {
        // "from" rectangle is wider, adjust "to" rectangle:
        qreal desiredWidth = (qreal)from.width() / (qreal)from.height() * (qreal)to.height();
        to.adjust(-(desiredWidth - to.width()) * 0.5, 0, (desiredWidth - to.width()) * 0.5, 0);
    } else if ((qreal)from.width() / (qreal)from.height() < (qreal)to.width() / (qreal)to.height()) {
        // "from" rectangle is higher, adjust "to" rectangle:
        qreal desiredHeight = (qreal)from.height() / (qreal)from.width() * (qreal)to.width();
        to.adjust(0, -(desiredHeight - to.height()) * 0.5, 0, (desiredHeight - to.height()) * 0.5);
    }
    QRect via = from | to;
    animation.stop();
    animation.setDuration(msecs);
    animation.setStartValue(from);
    if ((via == from) || (via == to)) {
        animation.setKeyValueAt(0.5, to);
    } else {
        animation.setKeyValueAt(0.5, via);
    }
    animation.setEndValue(to);
    animation.start();
}

void VisibleRectangleGraphicsView::animateToClientItem(GraphicsClientItem *item, int msecs)
{
    animateToVisibleSceneRect(item->getInnerItem()->sceneBoundingRect(), msecs);
}

void VisibleRectangleGraphicsView::animationFinished()
{
    animationFinished(this);
}
