#include "integraloscillator.h"

IntegralOscillator::IntegralOscillator(double frequencyModulationIntensity, double sampleRate, const QStringList &additionalInputPortNames) :
    Oscillator(frequencyModulationIntensity, sampleRate, additionalInputPortNames)
{
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(-1);
    xx.append(1);
    yy.append(1);
    integrals.append(PolynomialInterpolator(xx, yy));
}

PolynomialInterpolator * IntegralOscillator::getPolynomialInterpolator()
{
    return &integrals.first();
}

void IntegralOscillator::setPolynomialInterpolator(const PolynomialInterpolator &interpolator)
{
    integrals[0] = interpolator;
}

bool IntegralOscillator::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const Interpolator::ChangeControlPointEvent *event_ = dynamic_cast<const Interpolator::ChangeControlPointEvent*>(event)) {
        integrals.first().changeControlPoint(event_);
        return true;
    } else if (const Interpolator::AddControlPointsEvent *event_ = dynamic_cast<const Interpolator::AddControlPointsEvent*>(event)) {
        integrals.first().addControlPoints(event_);
        return true;
    } else if (const Interpolator::DeleteControlPointsEvent *event_ = dynamic_cast<const Interpolator::DeleteControlPointsEvent*>(event)) {
        integrals.first().deleteControlPoints(event_);
        return true;
    } else {
        return Oscillator::processEvent(event, time);
    }
}

double IntegralOscillator::valueAtPhase(double phase)
{
    // TODO
    return 0;
}
