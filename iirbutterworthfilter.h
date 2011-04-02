#ifndef IIRBUTTERWORTHFILTER_H
#define IIRBUTTERWORTHFILTER_H

/*
    Copyright 2011 Arne Jacobs

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "iirfilter.h"

class IirButterworthFilter : public IirFilter
{
public:
    enum Type {
        LOW_PASS,
        HIGH_PASS
    };

    IirButterworthFilter(double cutoffFrequencyInHertz, Type type = LOW_PASS, double sampleRate = 44100);

    virtual void setCutoffFrequency(double cutoffFrequencyInHertz, Type type);

    void setCutoffFrequency(double cutoffFrequencyInHertz);
    void setType(Type type);

    double getCutoffFrequency() const;
    Type getType() const;

private:
    double cutoffFrequency;
    Type type;
};

#endif // IIRBUTTERWORTHFILTER_H
