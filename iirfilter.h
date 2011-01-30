#ifndef IIRFILTER_H
#define IIRFILTER_H

#include <QVector>

class IIRFilter
{
public:
    IIRFilter(double sampleRate);
    IIRFilter(int feedForwardCoefficients, int feedBackCoefficients, double sampleRate);

    void debug();

    double getSampleRate() const;
    double getFrequencyInRadians(double frequencyInHertz) const;
    double filter(double x);
    void reset();
    double getSquaredAmplitudeResponse(double hertz);
    void addFeedForwardCoefficient(double c);
    void addFeedBackCoefficient(double c);

protected:
    void setFeedForwardCoefficient(int index, double c);
    void setFeedBackCoefficient(int index, double c);
    double getFeedForwardCoefficient(int index) const;
    double getFeedBackCoefficient(int index) const;

private:
    double sampleRate;
    QVector<double> feedForward, feedBack, x, y;
};

#endif // IIRFILTER_H
