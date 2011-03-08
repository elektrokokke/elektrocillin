#include "logarithmicinterpolator.h"
#include <QDebug>
#include <cmath>

LogarithmicInterpolator::LogarithmicInterpolator(double base_) :
    Interpolator(QVector<double>(), QVector<double>(), 2),
    base(base_)
{
}

LogarithmicInterpolator::LogarithmicInterpolator(const QVector<double> &xx, const QVector<double> &yy, double base_) :
    Interpolator(xx, yy, 2),
    base(base_)
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
    } else if (x == xx[j]) {
        return yy[j];
    } else if (x == xx[j + 1]) {
        return yy[j + 1];
    } else {
        double weight2 = (x - xx[j]) / (xx[j + 1] - xx[j]);
        if (base != 1) {
            weight2 = (1.0 - pow(base, weight2)) / (1.0 - base);
        }
        double weight1 = 1.0 - weight2;
        return yy[j] * weight1 + yy[j + 1] * weight2;
    }
}
