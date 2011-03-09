#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include "jackringbuffer.h"

class EventProcessor
{
public:
    virtual ~EventProcessor();
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time) = 0;
};

#endif // EVENTPROCESSOR_H
