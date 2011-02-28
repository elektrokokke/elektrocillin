#ifndef VISIBLERECTANGLEGRAPHICSVIEW_H
#define VISIBLERECTANGLEGRAPHICSVIEW_H

#include "graphicsclientitem2.h"
#include <QGraphicsView>
#include <QPropertyAnimation>

class VisibleRectangleGraphicsView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(QRectF visibleSceneRect READ visibleSceneRect WRITE setVisibleSceneRect);
public:
    explicit VisibleRectangleGraphicsView(QWidget *parent = 0);

    QRectF visibleSceneRect() const;
    void setVisibleSceneRect(const QRectF &rect);

    void animateToVisibleSceneRect(const QRectF &rect, int msecs = 750);
    void animateToClientItem(GraphicsClientItem2 *item, int msecs = 750);

signals:
    void animationFinished(QGraphicsView *view);

public slots:

protected:
    virtual void wheelEvent(QWheelEvent *event);

private:
    QPropertyAnimation animation;
    bool moving;
    QPointF mousePosWhenPressed, previousCenter;
    QCursor previousMouseCursor;

private slots:
    void animationFinished();
};

#endif // VISIBLERECTANGLEGRAPHICSVIEW_H
