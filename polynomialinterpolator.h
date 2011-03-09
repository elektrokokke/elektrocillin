#ifndef POLYNOMIALINTERPOLATOR_H
#define POLYNOMIALINTERPOLATOR_H

#include "interpolator.h"
#include "polynomial.h"

class PolynomialInterpolator : public Interpolator
{
public:
    PolynomialInterpolator();
    /**
      Creates a piecewise linear interpolator based on the given control points.
      */
    PolynomialInterpolator(const QVector<double> &xx, const QVector<double> &yy);

    void save(QDataStream &stream) const;
    void load(QDataStream &stream);

    /**
      Integrate the given polynomial segment-wise and stick the segments together
      such that they match at the segment bounds (i.e., the control points).
      */
    void integrate(const PolynomialInterpolator &polynomialInterpolator);
    /**
      Add a line to all segments such that the beginning of the first
      and the end of the last segment match.
      */
    void smoothen();

    virtual double interpolate(int jlo, double x);

    virtual void changeControlPoints(const QVector<double> &xx, const QVector<double> &yy);
    /**
      Calling this sets both polynomials around the given index
      to line segments, regardless of their previous degree.
      */
    virtual void changeControlPoint(int index, double x, double y);
    /**
      Calling these sets all polynomials to line segments, regardless of their previous degree.
      */
    virtual void addControlPoints(bool scaleX, bool scaleY, bool addAtStart, bool addAtEnd);
    virtual void deleteControlPoints(bool scaleX, bool scaleY, bool deleteAtStart, bool deleteAtEnd);
private:
    QVector<Polynomial<double> > polynomials;

    void initialize(const QVector<double> &xx, const QVector<double> &yy);
    void initializePolynomial(int index, double x1, double y1, double x2, double y2);
};

#endif // POLYNOMIALINTERPOLATOR_H
