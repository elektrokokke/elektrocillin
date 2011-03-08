#include "linearoscillator.h"
#include "cisi.h"
#include <cmath>
#include <QDebug>

LinearOscillator::LinearOscillator(double frequencyModulationIntensity, double sampleRate, const QStringList &additionalInputPortNames, int sincWindowSize_) :
    Oscillator(frequencyModulationIntensity, sampleRate, additionalInputPortNames),
    sincWindowSize(sincWindowSize_),
    siInterpolator(QVector<double>(), QVector<double>(), QVector<double>())
{
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(-1);
    xx.append(1);
    yy.append(1);
    interpolator.changeControlPoints(xx, yy);
    initializeSiInterpolator();
}

LinearOscillator::LinearOscillator(const LinearInterpolator &interpolator_, double frequencyModulationIntensity, double sampleRate, int sincWindowSize_) :
    Oscillator(frequencyModulationIntensity, sampleRate),
    interpolator(interpolator_),
    sincWindowSize(sincWindowSize_),
    siInterpolator(QVector<double>(), QVector<double>(), QVector<double>())
{
    initializeSiInterpolator();
}

const LinearInterpolator & LinearOscillator::getLinearInterpolator() const
{
    return interpolator;
}

void LinearOscillator::setLinearInterpolator(const LinearInterpolator &interpolator)
{
    Q_ASSERT(interpolator.getX()[0] == 0);
    Q_ASSERT(interpolator.getX().back() == 1);
    this->interpolator = interpolator;
}

void LinearOscillator::changeControlPoints(const QVector<double> &xx, const QVector<double> &yy)
{
    interpolator.changeControlPoints(xx, yy);
}

bool LinearOscillator::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const Interpolator::ChangeControlPointEvent *event_ = dynamic_cast<const Interpolator::ChangeControlPointEvent*>(event)) {
        interpolator.changeControlPoint(event_);
        return true;
    } else if (const Interpolator::AddControlPointsEvent *event_ = dynamic_cast<const Interpolator::AddControlPointsEvent*>(event)) {
        interpolator.addControlPoints(event_);
        return true;
    } else if (const Interpolator::DeleteControlPointsEvent *event_ = dynamic_cast<const Interpolator::DeleteControlPointsEvent*>(event)) {
        interpolator.deleteControlPoints(event_);
        return true;
    } else {
        return Oscillator::processEvent(event, time);
    }
}

double LinearOscillator::valueAtPhase(double phase)
{
//    return interpolator.evaluate(phase);
    double tmax = (double)sincWindowSize;
    double t1 = -tmax;
    double x1 = t1 * getNormalizedFrequency() + phase;
    for (; x1 < 0.0; ) {
        phase += 1;
        x1 += 1;
    }
    int i;
    interpolator.evaluate(x1, &i);
    double si1 = siInterpolator.evaluate(t1);
    double integral = 0.0;
    for (; t1 < tmax; ) {
        Q_ASSERT(i < interpolator.getX().size() - 1);
        double x2 = interpolator.getX()[i + 1];
        double t2 = (x2 - phase) / getNormalizedFrequency();
        if (t2 > tmax) {
            t2 = tmax;
            x2 = tmax * getNormalizedFrequency() + phase;
        }
        double si2 = siInterpolator.evaluate(t2);
        // ignore intervals that have zero width:
        if (x2 != x1) {
            double a = (interpolator.getY()[i + 1] - interpolator.getY()[i]) / (interpolator.getX()[i + 1] - interpolator.getX()[i]) / (M_PI * 2);
            double b = interpolator.getY()[i] - interpolator.getX()[i] * a;
            double integral1 = (a * phase + b) * si1 - a * getNormalizedFrequency() * 2 * cos(t1 * M_PI);
            double integral2 = (a * phase + b) * si2 - a * getNormalizedFrequency() * 2 * cos(t2 * M_PI);
            integral += integral2 - integral1;
        }
        i++;
        if (i == interpolator.getX().size() - 1) {
            i = 0;
            phase -= 1;
            x1 = 0.0;
        } else {
            x1 = x2;
        }
        t1 = t2;
        si1 = si2;
    }
    return integral / M_PI;
}

void LinearOscillator::initializeSiInterpolator()
{
    QVector<double> xx, yy;
    for (double t = -sincWindowSize; t <= sincWindowSize; t += 0.25) {
        xx.append(t);
        yy.append(Cisi::si(t * M_PI));
    }
    siInterpolator = CubicSplineInterpolator(xx, yy);
}
