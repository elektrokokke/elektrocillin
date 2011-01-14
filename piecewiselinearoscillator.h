#ifndef PIECEWISELINEAROSCILLATOR_H
#define PIECEWISELINEAROSCILLATOR_H

#include "oscillator.h"
#include <QList>

class PiecewiseLinearOscillator : public Oscillator
{
public:
    PiecewiseLinearOscillator();

    const QList<double> & getTimes() const;
    const QList<double> & getValues() const;

protected:
    double valueAtPhase(double phase);
    void addNode(double time, double value);

    QList<double> & getTimes();
    QList<double> & getValues();

private:
    QList<double> times, values;
    int from, to;
};

#endif // PIECEWISELINEAROSCILLATOR_H
