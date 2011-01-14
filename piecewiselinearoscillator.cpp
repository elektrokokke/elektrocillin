#include "piecewiselinearoscillator.h"
#include <cmath>

PiecewiseLinearOscillator::PiecewiseLinearOscillator() :
    from(0), to(1)
{
}

double PiecewiseLinearOscillator::valueAtPhase(double phase)
{
    if (times.isEmpty()) {
        return 0.0;
    } else if (times.size() == 1) {
        return values[0];
    } else {
        // find the right linear segment:
        double timeFrom = times[from];
        double timeTo = (to < from ? times[to] + 2.0 * M_PI : times[to]);
        for (; phase < timeFrom; ) {
            from = (from + times.size() - 1) % times.size();
            to = (to + times.size() - 1) % times.size();
            timeFrom = (to < from ? times[from] - 2.0 * M_PI : times[from]);
            timeTo = times[to];
        }
        for (; phase > timeTo; ) {
            from = (from + 1) % times.size();
            to = (to + 1) % times.size();
            timeFrom = times[from];
            timeTo = (to < from ? times[to] + 2.0 * M_PI : times[to]);
        }
        // interpolate between the end points of that segment:
        if (timeFrom == timeTo) {
            return values[from];
        } else {
            double weightFrom = (timeTo - phase) / (timeTo - timeFrom);
            double weightTo = 1.0 - weightFrom;
            return weightFrom * values[from] + weightTo * values[to];
        }
    }
}

void PiecewiseLinearOscillator::addNode(double time, double value)
{
    Q_ASSERT((time >= 0.0) && (time <= 2.0 * M_PI));
    Q_ASSERT(times.isEmpty() || (time >= times.last()));
    Q_ASSERT((value >= -1.0) && (value <= 1.0));
    times.append(time);
    values.append(value);
}

const QList<double> & PiecewiseLinearOscillator::getTimes() const
{
    return times;
}

const QList<double> & PiecewiseLinearOscillator::getValues() const
{
    return values;
}


QList<double> & PiecewiseLinearOscillator::getTimes()
{
    return times;
}

QList<double> & PiecewiseLinearOscillator::getValues()
{
    return values;
}
