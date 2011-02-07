#ifndef VISIBLERECTANGLEGRAPHICSVIEW_H
#define VISIBLERECTANGLEGRAPHICSVIEW_H

#include "graphicsclientitem.h"
#include <QGraphicsView>
#include <QPropertyAnimation>

class VisibleRectangleGraphicsView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(QRectF visibleSceneRect READ visibleSceneRect WRITE setVisibleSceneRect);
public:
    explicit VisibleRectangleGraphicsView(QWidget *parent = 0);
    explicit VisibleRectangleGraphicsView(QGraphicsScene *scene, QWidget *parent = 0);

    QRectF visibleSceneRect() const;
    void setVisibleSceneRect(const QRectF &rect);

    void animateToVisibleSceneRect(const QRectF &rect, int msecs = 750);
    void animateToClientItem(GraphicsClientItem *item, int msecs = 750);

signals:

public slots:

protected:
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );

private:
    QPropertyAnimation animation;
    bool moving;
    QPointF mousePosWhenPressed, previousCenter;
    QCursor previousMouseCursor;
};

#endif // VISIBLERECTANGLEGRAPHICSVIEW_H
