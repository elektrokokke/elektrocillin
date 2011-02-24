#include "jackringbuffer.h"
#include <QBuffer>

RingBuffer::RingBuffer(size_t ringBufferSize)
{
    ringBuffer = jack_ringbuffer_create(ringBufferSize * (sizeof(jack_nframes_t) + sizeof(RingBufferEvent*)));
    ringBufferReturn = jack_ringbuffer_create(ringBufferSize * sizeof(RingBufferEvent*));
}

RingBuffer::~RingBuffer()
{
    // delete all remaining event objects in the "return" ring buffer:
    for (; jack_ringbuffer_read_space(ringBufferReturn); ) {
        RingBufferEvent *event;
        jack_ringbuffer_read(ringBufferReturn, (char*)&event, sizeof(RingBufferEvent*));
        delete event;
    }
    jack_ringbuffer_free(ringBuffer);
    jack_ringbuffer_free(ringBufferReturn);
}

bool RingBuffer::hasEvents()
{
    return jack_ringbuffer_read_space(ringBuffer);
}

jack_nframes_t RingBuffer::peekEventTime()
{
    jack_nframes_t time;
    jack_ringbuffer_peek(ringBuffer, (char*)&time, sizeof(jack_nframes_t));
    return time;
}

bool RingBuffer::sendEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    // first delete all event objects in the "return" ring buffer:
    for (; jack_ringbuffer_read_space(ringBufferReturn); ) {
        RingBufferEvent *event;
        jack_ringbuffer_read(ringBufferReturn, (char*)&event, sizeof(RingBufferEvent*));
        delete event;
    }
    if (sizeof(jack_nframes_t) + sizeof(RingBufferEvent*) + sizeof(size_t) <= jack_ringbuffer_write_space(ringBuffer)) {
        // write the event time:
        jack_ringbuffer_write(ringBuffer, (const char*)&time, sizeof(jack_nframes_t));
        // write a pointer to event object:
        jack_ringbuffer_write(ringBuffer, (const char*)&event, sizeof(RingBufferEvent*));
        return true;
    } else {
        return false;
    }
}

RingBufferEvent * RingBuffer::readEvent(jack_nframes_t &time)
{
    // read the event time:
    jack_ringbuffer_read(ringBuffer, (char*)&time, sizeof(jack_nframes_t));
    // read the event object pointer from the ring buffer:
    RingBufferEvent *event;
    jack_ringbuffer_read(ringBuffer, (char*)&event, sizeof(RingBufferEvent*));
    return event;
}

RingBufferEvent * RingBuffer::readEvent()
{
    jack_nframes_t time;
    return readEvent(time);
}

void RingBuffer::returnEvent(RingBufferEvent *event)
{
    // write a pointer to the "return" ring buffer to be deleted with the next call to write():
    jack_ringbuffer_write(ringBufferReturn, (const char*)&event, sizeof(RingBufferEvent*));
}
