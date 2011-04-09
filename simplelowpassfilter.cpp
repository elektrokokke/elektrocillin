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

#include "simplelowpassfilter.h"

SimpleLowpassFilter::SimpleLowpassFilter(double a_, double b_) :
    previous(0.0),
    a(a_ / (a_ + b_)),
    b(b_ / (a_ + b_)),
    started(false)
{
}

double SimpleLowpassFilter::filter(double sample)
{
    if (!started) {
        started = true;
        previous = sample;
    }
    double filtered = a * sample + b * previous;
    previous = filtered;
    return filtered;
}

void SimpleLowpassFilter::reset()
{
    started = false;
}
