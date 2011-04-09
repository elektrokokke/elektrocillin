#ifndef ZPLANEFILTER_H
#define ZPLANEFILTER_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vector>
#include <complex>
#include "audioprocessor.h"
#include "frequencyresponse.h"

class ZPlaneFilter : public AudioProcessor, public FrequencyResponse
{
public:
    ZPlaneFilter();

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
