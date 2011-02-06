#ifndef VISIBLERECTANGLEGRAPHICSVIEW_H
#define VISIBLERECTANGLEGRAPHICSVIEW_H

#include <QGraphicsView>

class VisibleRectangleGraphicsView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(QRectF visibleSceneRect READ visibleSceneRect WRITE setVisibleSceneRect);
public:
    explicit VisibleRectangleGraphicsView(QWidget *parent = 0);
    explicit VisibleRectangleGraphicsView(QGraphicsScene *scene, QWidget *parent = 0);

    const QRectF & visibleSceneRect() const;
    void setVisibleSceneRect(const QRectF &rect);

    void animateToVisibleSceneRect(const QRectF &rect, int msecs = 750);

signals:

public slots:

private:
    QRectF visibleSceneRectangle;
};

#endif // VISIBLERECTANGLEGRAPHICSVIEW_H
