/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

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

#include "interpolatorprocessor.h"

void InterpolatorProcessor::changeControlPoint(Interpolator *interpolator, const ChangeControlPointEvent *event)
{
    interpolator->changeControlPoint(event->index, event->x, event->y);
}

void InterpolatorProcessor::addControlPoints(Interpolator *interpolator, const AddControlPointsEvent *event)
{
    interpolator->addControlPoints(event->scaleX, event->scaleY, event->addAtStart, event->addAtEnd);
}

void InterpolatorProcessor::deleteControlPoints(Interpolator *interpolator, const DeleteControlPointsEvent *event)
{
    interpolator->deleteControlPoints(event->scaleX, event->scaleY, event->deleteAtStart, event->deleteAtEnd);
}

void InterpolatorProcessor::processInterpolatorEvent(Interpolator *interpolator, const InterpolatorEvent *event)
{
    if (const ChangeControlPointEvent *event_ = dynamic_cast<const ChangeControlPointEvent*>(event)) {
        changeControlPoint(interpolator, event_);
    } else if (const AddControlPointsEvent *event_ = dynamic_cast<const AddControlPointsEvent*>(event)) {
        addControlPoints(interpolator, event_);
    } else if (const DeleteControlPointsEvent *event_ = dynamic_cast<const DeleteControlPointsEvent*>(event)) {
        deleteControlPoints(interpolator, event_);
    }
}
