#ifndef ZPLANEWIDGET_H
#define ZPLANEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QGraphicsLineItem>
#include "zplanefilter.h"

namespace Ui {
    class ZPlaneWidget;
}

class QGraphicsEllipseItem;
class QGraphicsRectItem;
class QGraphicsScene;

class ZPlaneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ZPlaneWidget(QWidget *parent = 0);
    ~ZPlaneWidget();

private:
    Ui::ZPlaneWidget *ui;
    int xscale, yscale;
    ZPlaneFilter filter2pole;
    QVector<double> squaredAmplitudeResponse2pole;
    QGraphicsScene *scene;
    QVector<QGraphicsLineItem*> lines2pole;
    QVector<QGraphicsEllipseItem*> zeroItems;
    QVector<QGraphicsRectItem*> poleItems;

    void drawButterworthPoleTrail();

private slots:
    void updateFrequencyResponse2pole();
};

#endif // ZPLANEWIDGET_H
