#include "linearinterpolator.h"
#include <QDebug>

LinearInterpolator::LinearInterpolator() :
    Interpolator(QVector<double>(), QVector<double>(), 2)
{
}

LinearInterpolator::LinearInterpolator(const QVector<double> &xx, const QVector<double> &yy) :
    Interpolator(xx, yy, 2)
{
}

void LinearInterpolator::save(QDataStream &stream) const
{
    stream << xx << yy;
}

void LinearInterpolator::load(QDataStream &stream)
{
    stream >> xx >> yy;
}

const QVector<double> & LinearInterpolator::getX() const
{
    return Interpolator::getX();
}

const QVector<double> & LinearInterpolator::getY() const
{
    return Interpolator::getY();
}

QVector<double> & LinearInterpolator::getX()
{
    return xx;
}

QVector<double> & LinearInterpolator::getY()
{
    return yy;
}

/**
  Comments from "Numerical Recipes"

  Piecewise linear interpolation object. Construct with x and y vectors,
  then call interpolate for interpolated values.
  */
double LinearInterpolator::interpolate(int j, double x)
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
    if (xx[j] == xx[j + 1]) {   // Table is defective, but we can recover.
        return yy[j];
    } else {
        return yy[j] + ((x - xx[j]) / (xx[j + 1] - xx[j])) * (yy[j + 1] - yy[j]);
    }
}

void LinearInterpolator::processEvent(const Interpolator::ChangeControlPointEvent *event)
{
    Q_ASSERT((event->index >= 0) && (event->index < xx.size()));
    xx[event->index] = event->x;
    yy[event->index] = event->y;
}

void LinearInterpolator::processEvent(const Interpolator::ChangeAllControlPointsEvent *event)
{
    xx = event->xx;
    yy = event->yy;
}
