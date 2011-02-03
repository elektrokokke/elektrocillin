#ifndef ZPLANEFILTER_H
#define ZPLANEFILTER_H

#include <vector>
#include <complex>
#include "audioprocessor.h"
#include "frequencyresponse.h"

class ZPlaneFilter : public AudioProcessor, public FrequencyResponse
{
public:
    ZPlaneFilter(double sampleRate = 44100);

    // reimplemented from AudioProcessor:
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplenented from FrequencyResponse:
    double getSquaredAmplitudeResponse(double hertz);

    void reset();

    void addPole(const std::complex<double> &pole);
    void addZero(const std::complex<double> &zero);

    size_t poleCount() const;
    size_t zeroCount() const;

    std::complex<double> & pole(size_t i);
    std::complex<double> & zero(size_t i);

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
