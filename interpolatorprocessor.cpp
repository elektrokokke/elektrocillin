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
