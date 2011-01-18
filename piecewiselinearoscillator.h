#ifndef PIECEWISELINEAROSCILLATOR_H
#define PIECEWISELINEAROSCILLATOR_H

#include "oscillator.h"
#include <QList>
#include <QPointF>

class PiecewiseLinearOscillator : public Oscillator
{
public:
    const QList<QPointF> & getNodes() const;
    QPointF getNode(int index) const;

protected:
    double valueAtPhase(double phase);
    void addNode(const QPointF &node);

    QList<QPointF> & getNodes();

private:
    QList<QPointF> nodes;

    static double interpolate(const QPointF &n0, const QPointF &n1, double x);
};

#endif // PIECEWISELINEAROSCILLATOR_H
