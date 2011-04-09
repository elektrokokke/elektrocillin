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

#include "jackringbuffer.h"
#include <QBuffer>

RingBuffer::RingBuffer(size_t ringBufferSize)
{
    ringBuffer = jack_ringbuffer_create(ringBufferSize * (sizeof(jack_nframes_t) + sizeof(RingBufferEvent*)));
    ringBufferReturn = jack_ringbuffer_create(ringBufferSize * sizeof(RingBufferEvent*));
}

RingBuffer::~RingBuffer()
{
    // delete all remaining undeleted event objects:
    for (; undeletedEvents.size(); ) {
        RingBufferEvent *event = *undeletedEvents.begin();
        undeletedEvents.erase(undeletedEvents.begin());
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

bool RingBuffer::sendEvent(RingBufferEvent *event, jack_nframes_t time)
{
    // first delete all event objects in the "return" ring buffer:
    for (; jack_ringbuffer_read_space(ringBufferReturn); ) {
        RingBufferEvent *event;
        jack_ringbuffer_read(ringBufferReturn, (char*)&event, sizeof(RingBufferEvent*));
        undeletedEvents.remove(event);
        delete event;
    }
    if (sizeof(jack_nframes_t) + sizeof(RingBufferEvent*) + sizeof(size_t) <= jack_ringbuffer_write_space(ringBuffer)) {
        undeletedEvents.insert(event);
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
