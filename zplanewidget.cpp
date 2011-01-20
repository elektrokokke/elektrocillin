#include "zplanewidget.h"
#include "ui_zplanewidget.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QDebug>
#include <cmath>

ZPlaneWidget::ZPlaneWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZPlaneWidget),
    xscale(5),
    yscale(10)
{
    ui->setupUi(this);

    int max = 80;
    squaredAmplitudeResponse2pole.resize(257);
    squaredAmplitudeResponse4pole.resize(257);

    // setup the z-plane filter:
    filter2pole.addPole(std::complex<double>());
    filter2pole.addPole(std::complex<double>());
    // set the zeros such as to normalize the resonance gain:
    // (see http://www.dsprelated.com/dspbooks/filters/Constant_Resonance_Gain.html)
    filter2pole.addZero(std::complex<double>(1.0, 0.0));
    filter2pole.addZero(std::complex<double>(-1.0, 0.0));

    filter4pole.addPole(std::complex<double>());
    filter4pole.addPole(std::complex<double>());
    filter4pole.addPole(std::complex<double>());
    filter4pole.addPole(std::complex<double>());
    filter4pole.addZero(std::complex<double>(1.0, 0.0));
    filter4pole.addZero(std::complex<double>(1.0, 0.0));
    filter4pole.addZero(std::complex<double>(-1.0, 0.0));
    filter4pole.addZero(std::complex<double>(-1.0, 0.0));

    // setup the QGraphicsScene:
    QGraphicsScene *scene = new QGraphicsScene(this);
    for (int i = 0; i < squaredAmplitudeResponse2pole.size() - 1; i++) {
        lines2pole.append(scene->addLine(i * xscale, 0, (i+1) * xscale, 0, QPen(Qt::red)));
        lines4pole.append(scene->addLine(i * xscale, 0, (i+1) * xscale, 0, QPen(Qt::green)));
    }
    scene->addLine(0, 0, squaredAmplitudeResponse2pole.size() * xscale, 0);
    scene->addLine(0, max * yscale, 0, -max * yscale);
    for (int i = -max; i <= max; i++) {
        scene->addText(QString::number(i))->setPos(-20, -i * yscale);
        scene->addLine(-5, -i * yscale, 0, -i * yscale);
    }

    // be notified when the user changes the filter parameters:
    QObject::connect(ui->spinBoxFrequency, SIGNAL(valueChanged(double)), this, SLOT(updateFrequencyResponse2pole()));
    QObject::connect(ui->spinBoxQFactor, SIGNAL(valueChanged(double)), this, SLOT(updateFrequencyResponse2pole()));
    QObject::connect(ui->spinBoxFrequency_2, SIGNAL(valueChanged(double)), this, SLOT(updateFrequencyResponse4pole()));
    QObject::connect(ui->spinBoxQFactor_2, SIGNAL(valueChanged(double)), this, SLOT(updateFrequencyResponse4pole()));
    QObject::connect(ui->spinBoxFrequency_3, SIGNAL(valueChanged(double)), this, SLOT(updateFrequencyResponse4pole()));
    QObject::connect(ui->spinBoxQFactor_3, SIGNAL(valueChanged(double)), this, SLOT(updateFrequencyResponse4pole()));

    updateFrequencyResponse2pole();
    updateFrequencyResponse4pole();

    ui->graphicsView->setScene(scene);
}

ZPlaneWidget::~ZPlaneWidget()
{
    delete ui;
}

void ZPlaneWidget::updateFrequencyResponse2pole()
{
    // update the z-plane filter with the user-defined parameters:
    filter2pole.pole(0) = std::polar(ui->spinBoxQFactor->value(), ui->spinBoxFrequency->value());
    // keep conjugate pairs of poles:
    filter2pole.pole(1) = std::conj(filter2pole.pole(0));
    // compute the frequency response:
    for (int i = 0; i < squaredAmplitudeResponse2pole.size(); i++) {
        squaredAmplitudeResponse2pole[i] = filter2pole.squaredAmplitudeResponse((double)i * M_PI / (double)(squaredAmplitudeResponse2pole.size() - 1));
        if (squaredAmplitudeResponse2pole[i] == 0.0) {
            squaredAmplitudeResponse2pole[i] = -80.0;
        } else {
            squaredAmplitudeResponse2pole[i] = std::max(-80.0, ZPlaneFilter::convertPowerToDecibel(squaredAmplitudeResponse2pole[i]));
        }
    }
    // update the graphics view:
    for (int i = 0; i < squaredAmplitudeResponse2pole.size() - 1; i++) {
        lines2pole[i]->setLine(i * xscale, -squaredAmplitudeResponse2pole[i] * yscale, (i+1) * xscale, -squaredAmplitudeResponse2pole[i+1] * yscale);
    }
}

void ZPlaneWidget::updateFrequencyResponse4pole()
{
    // update the z-plane filter with the user-defined parameters:
    filter4pole.pole(0) = std::polar(ui->spinBoxQFactor_2->value(), ui->spinBoxFrequency_2->value());
    filter4pole.pole(1) = std::polar(ui->spinBoxQFactor_2->value(), ui->spinBoxFrequency_2->value());
    // keep conjugate pairs of poles:
    filter4pole.pole(2) = std::conj(filter4pole.pole(0));
    filter4pole.pole(3) = std::conj(filter4pole.pole(1));
    // compute the frequency response:
    for (int i = 0; i < squaredAmplitudeResponse4pole.size(); i++) {
        squaredAmplitudeResponse4pole[i] = filter4pole.squaredAmplitudeResponse((double)i * M_PI / (double)(squaredAmplitudeResponse4pole.size() - 1));
        if (squaredAmplitudeResponse4pole[i] == 0.0) {
            squaredAmplitudeResponse4pole[i] = -80.0;
        } else {
            squaredAmplitudeResponse4pole[i] = std::max(-80.0, ZPlaneFilter::convertPowerToDecibel(squaredAmplitudeResponse4pole[i]));
        }
    }
    // update the graphics view:
    for (int i = 0; i < squaredAmplitudeResponse4pole.size() - 1; i++) {
        lines4pole[i]->setLine(i * xscale, -squaredAmplitudeResponse4pole[i] * yscale, (i+1) * xscale, -squaredAmplitudeResponse4pole[i+1] * yscale);
    }
}
