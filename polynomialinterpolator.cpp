#include "polynomialinterpolator.h"

PolynomialInterpolator::PolynomialInterpolator() :
    Interpolator(QVector<double>(), 2)
{
}

PolynomialInterpolator::PolynomialInterpolator(const QVector<double> &xx, const QVector<double> &yy) :
    Interpolator(xx, 2)
{
    initialize(xx, yy);
}

void PolynomialInterpolator::save(QDataStream &stream) const
{
    int size = polynomials.size();
    stream << size;
    for (int i = 0; i < size; i++) {
        int polynomialSize = polynomials[i].size();
        stream << polynomialSize;
        for (int j = 0; j < polynomialSize; j++) {
            stream << polynomials[i][j];
        }
    }
}

void PolynomialInterpolator::load(QDataStream &stream)
{
    int size;
    stream >> size;
    polynomials.resize(size);
    for (int i = 0; i < size; i++) {
        int polynomialSize;
        stream >> polynomialSize;
        polynomials[i].resize(polynomialSize);
        for (int j = 0; j < polynomialSize; j++) {
            stream >> polynomials[i][j];
        }
    }
}

void PolynomialInterpolator::integrate(const PolynomialInterpolator &polynomialInterpolator)
{
    xx = polynomialInterpolator.getX();
    polynomials.resize(polynomialInterpolator.polynomials.size());
    // first integrate all segments independently:
    for (int i = 0; i < polynomials.size(); i++) {
        polynomials[i].integrate(polynomialInterpolator.polynomials[i]);
    }
    // now make their ends match:
    for (int i = 1; i < polynomials.size(); i++) {
        polynomials[i].adjustConstantToIntersect(xx[i], polynomials[i - 1].evaluate(xx[i]));
    }
}

void PolynomialInterpolator::smoothen()
{
    double x1 = xx.first();
    double x2 = xx.last();
    if (x1 != x2) {
        // get the value at the start of the first segment:
        double startValue = polynomials.first().evaluate(x1);
        // get the value at the end of the last segment:
        double endValue = polynomials.last().evaluate(x2);
        // the line to compensate should be zero at the start and (startValue - endValue) at the end:
        double y1 = 0;
        double y2 = startValue - endValue;
        // compute the coefficients of the line between both points:
        double a = (y2 - y1) / (x2 - x1);
        double b = y1 - a * x1;
        Polynomial<double> lineToAdd(b, a);
        for (int i = 0; i < polynomials.size(); i++) {
            polynomials[i] += lineToAdd;
        }
    }
}

double PolynomialInterpolator::interpolate(int jlo, double x)
{
    if (jlo < 0) {
        return polynomials.first().evaluate(x);
    } else if (jlo >= polynomials.size()) {
        return polynomials.back().evaluate(x);
    } else {
        return polynomials[jlo].evaluate(x);
    }
}

void PolynomialInterpolator::processEvent(const ChangeControlPointEvent *event)
{
    Q_ASSERT((event->index >= 0) && (event->index < xx.size()));
    Q_ASSERT((event->index == 0) || (xx[event->index] >= xx[event->index - 1]));
    Q_ASSERT((event->index == xx.size() - 1) || (xx[event->index] <= xx[event->index + 1]));
    xx[event->index] = event->x;
    if (event->index > 0) {
        double x = xx[event->index - 1];
        double y = polynomials[event->index - 1].evaluate(x);
        initializePolynomial(event->index - 1, x, y, event->x, event->y);
    }
    if (event->index < xx.size() - 1) {
        double x = xx[event->index + 1];
        double y = polynomials[event->index].evaluate(x);
        initializePolynomial(event->index, event->x, event->y, x, y);
    }
}

void PolynomialInterpolator::processEvent(const ChangeAllControlPointsEvent *event)
{
    initialize(event->xx, event->yy);
}

void PolynomialInterpolator::initialize(const QVector<double> &xx, const QVector<double> &yy)
{
    polynomials.resize(xx.size() - 1);
    this->xx = xx;
    for (int i = 0; i < xx.size() - 1; i++) {
        initializePolynomial(i, xx[i], yy[i], xx[i + 1], yy[i + 1]);
    }
}

void PolynomialInterpolator::initializePolynomial(int i, double x1, double y1, double x2, double y2)
{
    // initialize the single-degree polynomials based on the given control points:
    if (x1 == x2) {
        polynomials[i].resize(1);
        polynomials[i].at(0) = x1;
    } else {
        double a = (y2 - y1) / (x2 - x1);
        double b = y1 - a * x1;
        polynomials[i].resize(2);
        polynomials[i].at(0) = b;
        polynomials[i].at(1) = a;
    }
}
