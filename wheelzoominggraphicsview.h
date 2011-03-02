#ifndef VISIBLERECTANGLEGRAPHICSVIEW_H
#define VISIBLERECTANGLEGRAPHICSVIEW_H

#include <QGraphicsView>

class WheelZoomingGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit WheelZoomingGraphicsView(QWidget *parent = 0);

protected:
    virtual void wheelEvent(QWheelEvent *event);
};

#endif // VISIBLERECTANGLEGRAPHICSVIEW_H
