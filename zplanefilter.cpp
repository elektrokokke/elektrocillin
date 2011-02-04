#include "zplanefilter.h"
#include "polynomial.h"
#include <cmath>
#include <QDebug>

ZPlaneFilter::ZPlaneFilter(double sampleRate) :
    AudioProcessor(QStringList("audio_in"), QStringList("filtered_out"), sampleRate),
    tx(0),
    ty(0)
{
}

// reimplenented from FrequencyResponse:
void ZPlaneFilter::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    // set the current x:
    x[tx] = inputs[0];
    // compute current y:
    y[ty] = 0.0;
    for (size_t i = 0; i < x.size(); i++) {
        y[ty] += x[(tx + i) % x.size()] * feedforwardCoefficients[i];
    }
    for (size_t i = 1; i < y.size(); i++) {
        y[ty] -= y[(ty + i) % y.size()] * feedbackCoefficients[i];
    }
    outputs[0] = y[ty];
    // advance the position for previous results:
    tx = (tx + x.size() - 1) % x.size();
    ty = (ty + y.size() - 1) % y.size();
}

double ZPlaneFilter::getSquaredAmplitudeResponse(double hertz)
{
    // compute the squared amplitude response (power) from the frequency response:
    return std::norm(frequencyResponse(convertHertzToRadians(hertz)));
}

void ZPlaneFilter::reset()
{
    for (size_t i = 0; i < x.size(); i++) {
        x[i] = 0.0;
    }
    for (size_t i = 0; i < y.size(); i++) {
        y[i] = 0.0;
    }
    tx = ty = 0;
}

void ZPlaneFilter::addPole(const std::complex<double> &pole)
{
    poles.push_back(pole);
}

void ZPlaneFilter::addZero(const std::complex<double> &zero)
{
    zeros.push_back(zero);
}

size_t ZPlaneFilter::poleCount() const
{
    return poles.size();
}

size_t ZPlaneFilter::zeroCount() const
{
    return zeros.size();
}

std::complex<double> & ZPlaneFilter::pole(size_t i)
{
    return poles[i];
}

std::complex<double> & ZPlaneFilter::zero(size_t i)
{
    return zeros[i];
}

std::complex<double> ZPlaneFilter::frequencyResponse(double frequencyInRadians)
{
    // convert the frequency to a complex number on the unit circle:
    return frequencyResponse(std::polar(1.0, frequencyInRadians));
}

std::complex<double> ZPlaneFilter::frequencyResponse(const std::complex<double> z)
{
    // evaluate based on the pole/zero representation:
    std::complex<double> result = 0.0;
    {
        std::complex<double> n(1.0, 0.0), d(1.0, 0.0);
        for (size_t i = 0; i < zeros.size(); i++) {
            n *= (z - zeros[i]);
        }
        for (size_t i = 0; i < poles.size(); i++) {
            d *= (z - poles[i]);
        }
        //qDebug() << "simple computation" << n.real() << n.imag() << "/" << d.real() << d.imag();
        result = n/d;
    }
    {
        // compute numerator and denominator:
        Polynomial<std::complex<double> > numerator(1.0);
        for (size_t i = 0; i < zeros.size(); i++) {
            Polynomial<std::complex<double> > zero(1.0, -zeros[i]);
            numerator *= zero;
        }
        Polynomial<std::complex<double> > denominator(1.0);
        for (size_t i = 0; i < poles.size(); i++) {
            Polynomial<std::complex<double> > pole(1.0, -poles[i]);
            denominator *= pole;
        }
        std::complex<double> n = numerator.evaluate(1.0/z);
        std::complex<double> d = denominator.evaluate(1.0/z);
        //qDebug() << "computation by polynomials" << n.real() << n.imag() << "/" << d.real() << d.imag();
    }
    return result;
}

void ZPlaneFilter::computeCoefficients()
{
    x.clear();
    y.clear();
    // compute numerator and denominator:
    Polynomial<std::complex<double> > numerator(1.0);
    for (size_t i = 0; i < zeros.size(); i++) {
        //qDebug() << "zero" << i;
        for (size_t j = 0; j < numerator.size(); j++) {
            //qDebug() << "(" << numerator[j].real() << "," << numerator[j].imag() << "i )" << "* x^" << j;
        }
        //qDebug() << "*";
        Polynomial<std::complex<double> > zero(1.0, -zeros[i]);
        for (size_t j = 0; j < zero.size(); j++) {
            //qDebug() << "(" << zero[j].real() << "," << zero[j].imag() << "i )" << "* x^" << j;
        }
        //qDebug() << "=";
        numerator *= zero;
        for (size_t j = 0; j < numerator.size(); j++) {
            //qDebug() << "(" << numerator[j].real() << "," << numerator[j].imag() << "i )" << "* x^" << j;
        }
    }
    Polynomial<std::complex<double> > denominator(1.0);
    for (size_t i = 0; i < poles.size(); i++) {
        //qDebug() << "pole" << i;
        for (size_t j = 0; j < denominator.size(); j++) {
            //qDebug() << "(" << denominator[j].real() << "," << denominator[j].imag() << "i )" << "* x^" << j;
        }
        //qDebug() << "*";
        Polynomial<std::complex<double> > pole(1.0, -poles[i]);
        for (size_t j = 0; j < pole.size(); j++) {
            //qDebug() << "(" << pole[j].real() << "," << pole[j].imag() << "i )" << "* x^" << j;
        }
        //qDebug() << "=";
        denominator *= pole;
        for (size_t j = 0; j < denominator.size(); j++) {
            //qDebug() << "(" << denominator[j].real() << "," << denominator[j].imag() << "i )" << "* x^" << j;
        }
    }
    // let DC always be 1, i.e., scale feed forward coefficients accordingly:
    double dc = sqrt(getSquaredAmplitudeResponse(0.0));
    numerator *= 1.0 / dc;
    qDebug() << "numerator";
    for (size_t i = 0; i < numerator.size(); i++) {
        qDebug() << numerator[i].real() << numerator[i].imag();
        feedforwardCoefficients.push_back(numerator[i].real());
        x.push_back(0.0);
    }
    qDebug() << "denominator";
    for (size_t i = 0; i < denominator.size(); i++) {
        qDebug() << denominator[i].real() << denominator[i].imag();
        feedbackCoefficients.push_back(denominator[i].real());
        y.push_back(0.0);
    }
}

double ZPlaneFilter::convertPowerToDecibel(double power)
{
    // follow the 10 * log10 rule:
    return 10.0 * log(power) / log(10.0);
}
