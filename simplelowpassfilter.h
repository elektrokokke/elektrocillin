#ifndef SIMPLELOWPASSFILTER_H
#define SIMPLELOWPASSFILTER_H

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

class SimpleLowpassFilter
{
public:
    SimpleLowpassFilter(double a = 0.1, double b = 0.9);

    double filter(double sample);

    void reset();

private:
    double previous, a, b;
    bool started;
};

#endif // SIMPLELOWPASSFILTER_H
