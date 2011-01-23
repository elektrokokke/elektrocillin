#ifndef ZPLANEFILTER_H
#define ZPLANEFILTER_H

#include <vector>
#include <complex>

class ZPlaneFilter
{
public:
    ZPlaneFilter();

    double filter(double x0);
    void reset();

    void addPole(const std::complex<double> &pole);
    void addZero(const std::complex<double> &zero);

    size_t poleCount() const;
    size_t zeroCount() const;

    std::complex<double> & pole(size_t i);
    std::complex<double> & zero(size_t i);

    double squaredAmplitudeResponse(double frequencyInRadians);
    std::complex<double> frequencyResponse(double frequencyInRadians);
    std::complex<double> frequencyResponse(const std::complex<double> z);

    void computeCoefficients();

    static double convertPowerToDecibel(double power);
private:
    std::vector<std::complex<double> > poles, zeros;
    //std::vector<std::complex<double> > feedforwardCoefficients, feedbackCoefficients, x, y;
    std::vector<double> feedforwardCoefficients, feedbackCoefficients, x, y;
    size_t tx, ty;

};

#endif // ZPLANEFILTER_H
