#ifndef SINCFILTER_H
#define SINCFILTER_H

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

#include "audioprocessor.h"

class SincFilter : public AudioProcessor
{
public:
    SincFilter();
    /**
      @param size determines the number of filter coefficients. Actual filter size
        will be size * 2 + 1.
      */
    SincFilter(int size, double frequency);

    virtual void setSampleRate(double sampleRate);

    void feed(double input);
    double process();

    /**
      A short cut for feed() followed by process().
      */
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
private:
    int index, size;
    double frequency;
    QVector<double> coefficients, previousInputs;
};

#endif // SINCFILTER_H
