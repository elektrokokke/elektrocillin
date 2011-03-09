#ifndef INTERPOLATORPROCESSOR_H
#define INTERPOLATORPROCESSOR_H

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
