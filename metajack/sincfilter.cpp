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

#include "sincfilter.h"
#include <math.h>

SincFilter::SincFilter() :
        AudioProcessor(QStringList(), QStringList())
{}

SincFilter::SincFilter(int size_, double frequency_) :
    AudioProcessor(QStringList("Audio in"), QStringList("Filtered out")),
    index(0),
    size(size_),
    frequency(frequency_),
    coefficients(size * 2 + 1),
    previousInputs(size * 2 + 1, 0)
{
}

void SincFilter::setSampleRate(double sampleRate)
{
    AudioProcessor::setSampleRate(sampleRate);
    // compute the filter coefficients:
    double sum = 1;
    coefficients[size] = 1;
    for (int i = 1; i <= size; i++) {
        double x = (double)i * 2 * M_PI * frequency / sampleRate;
        double sinc = sin(x) / x;
        coefficients[size + i] = coefficients[size - i] = sinc;
        sum += sinc + sinc;
    }
    double factor = 1.0 / sum;
    for (int i = 0; i < coefficients.size(); i++) {
        coefficients[i] *= factor;
    }
}

void SincFilter::feed(double input)
{
    previousInputs[index] = input;
    index = (index + 1) % previousInputs.size();
}

double SincFilter::process()
{
    double sum = 0;
    for (int i = 0; i < coefficients.size(); i++) {
        sum += coefficients[i] * previousInputs[(index + i) % previousInputs.size()];
    }
    return sum;
}

void SincFilter::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    feed(inputs[0]);
    outputs[0] = process();
}
