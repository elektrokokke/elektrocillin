#include "logarithmicinterpolator.h"
#include <QDebug>

LogarithmicInterpolator::LogarithmicInterpolator() :
    Interpolator(QVector<double>(), 2)
{
}

LogarithmicInterpolator::LogarithmicInterpolator(const QVector<double> &xx, const QVector<double> &yy_) :
    Interpolator(xx, 2),
    yy(yy_)
{
}

void LogarithmicInterpolator::save(QDataStream &stream) const
{
    stream << xx << yy;
}

void LogarithmicInterpolator::load(QDataStream &stream)
{
    stream >> xx >> yy;
}

const QVector<double> & LogarithmicInterpolator::getX() const
{
    return Interpolator::getX();
}

const QVector<double> & LogarithmicInterpolator::getY() const
{
    return yy;
}

QVector<double> & LogarithmicInterpolator::getX()
{
    return xx;
}

QVector<double> & LogarithmicInterpolator::getY()
{
    return yy;
}

double LogarithmicInterpolator::interpolate(int j, double x)
{
    Q_ASSERT(xx.size() >= 2);
    if (j < 0) {
        j = 0;
    }
    if (j >= xx.size() - 1) {
        if (xx[xx.size() - 1] == xx[xx.size() - 2]) {
            return yy.last();
        } else {
            j = xx.size() - 2;
        }
    }
    if (xx[j] == xx[j + 1]) {
        return yy[j];
    } else {
        double a = (1.0 - exp(1.0)) / (xx[j] - xx[j + 1]);
        double b = 1.0 - a * xx[j];
        double weight2 = a * x + b;
        double weight1 = 1.0 - weight2;
        return yy[j] * weight1 + yy[j + 1] * weight2;
    }
}

void LogarithmicInterpolator::processEvent(const Interpolator::ChangeControlPointEvent *event)
{
    Q_ASSERT((event->index >= 0) && (event->index < xx.size()));
    xx[event->index] = event->x;
    yy[event->index] = event->y;
}

void LogarithmicInterpolator::processEvent(const Interpolator::ChangeAllControlPointsEvent *event)
{
    xx = event->xx;
    yy = event->yy;
}
