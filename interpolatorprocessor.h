#ifndef INTERPOLATORPROCESSOR_H
#define INTERPOLATORPROCESSOR_H

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

#include "interpolator.h"
#include "eventprocessor.h"

class InterpolatorProcessor
{
public:
    class InterpolatorEvent : public RingBufferEvent
    {
    public:
        virtual ~InterpolatorEvent() {}
    };
    class ChangeControlPointEvent : public InterpolatorEvent
    {
    public:
        ChangeControlPointEvent(int index_, double x_, double y_) :
            index(index_), x(x_), y(y_)
        {}
        int index;
        double x, y;
    };
    class AddControlPointsEvent : public InterpolatorEvent
    {
    public:
        AddControlPointsEvent(bool scaleX_, bool scaleY_, bool addAtStart_, bool addAtEnd_) :
            scaleX(scaleX_), scaleY(scaleY_), addAtStart(addAtStart_), addAtEnd(addAtEnd_)
        {}
        bool scaleX, scaleY, addAtStart, addAtEnd;
    };
    class DeleteControlPointsEvent : public InterpolatorEvent
    {
    public:
        DeleteControlPointsEvent(bool scaleX_, bool scaleY_, bool deleteAtStart_, bool deleteAtEnd_) :
            scaleX(scaleX_), scaleY(scaleY_), deleteAtStart(deleteAtStart_), deleteAtEnd(deleteAtEnd_)
        {}
        bool scaleX, scaleY, deleteAtStart, deleteAtEnd;
    };

    virtual void changeControlPoint(Interpolator *interpolator, const ChangeControlPointEvent *event);
    virtual void addControlPoints(Interpolator *interpolator, const AddControlPointsEvent *event);
    virtual void deleteControlPoints(Interpolator *interpolator, const DeleteControlPointsEvent *event);

    virtual void processInterpolatorEvent(Interpolator *interpolator, const InterpolatorEvent *event);
};

#endif // INTERPOLATORPROCESSOR_H
