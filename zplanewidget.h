#ifndef ZPLANEWIDGET_H
#define ZPLANEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QGraphicsLineItem>
#include "zplanefilter.h"

namespace Ui {
    class ZPlaneWidget;
}

class ZPlaneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ZPlaneWidget(QWidget *parent = 0);
    ~ZPlaneWidget();

private:
    Ui::ZPlaneWidget *ui;
    int xscale, yscale;
    ZPlaneFilter filter2pole, filter4pole;
    QVector<double> squaredAmplitudeResponse2pole, squaredAmplitudeResponse4pole;
    QVector<QGraphicsLineItem*> lines2pole, lines4pole;

private slots:
    void updateFrequencyResponse2pole();
    void updateFrequencyResponse4pole();
};

#endif // ZPLANEWIDGET_H
