#include "jackringbuffer.h"
#include <QBuffer>

RingBuffer::RingBuffer(size_t ringBufferSize)
{
    ringBuffer = jack_ringbuffer_create(ringBufferSize);
}

RingBuffer::~RingBuffer()
{
    jack_ringbuffer_free(ringBuffer);
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

bool RingBuffer::write(const RingBufferEvent *event, jack_nframes_t time)
{
    // write the event to our byte array buffer first:
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);
    event->write(stream);
    if (buffer.pos() + sizeof(jack_nframes_t) + sizeof(QMetaObject*) + sizeof(size_t) <= jack_ringbuffer_write_space(ringBuffer)) {
        // it fits into the ring buffer...
        // write the event time:
        jack_ringbuffer_write(ringBuffer, (const char*)&time, sizeof(jack_nframes_t));
        // write a pointer to the event's meta object:
        const QMetaObject *metaObject = event->metaObject();
        jack_ringbuffer_write(ringBuffer, (const char*)&metaObject, sizeof(QMetaObject*));
        // write the event to the ring buffer:
        size_t size = byteArray.size();
        jack_ringbuffer_write(ringBuffer, (const char*)&size, sizeof(size_t));
        jack_ringbuffer_write(ringBuffer, byteArray.data(), size);
        return true;
    } else {
        return false;
    }
}

RingBufferEvent * RingBuffer::read(jack_nframes_t &time)
{
    // read the event time:
    jack_ringbuffer_read(ringBuffer, (char*)&time, sizeof(jack_nframes_t));
    // read a pointer to the event's meza object:
    const QMetaObject *metaObject;
    jack_ringbuffer_read(ringBuffer, (char*)&metaObject, sizeof(QMetaObject*));
    // create a new event object:
    RingBufferEvent *event = (RingBufferEvent*)metaObject->newInstance();
    // read the event from the ring buffer to our byte arry buffer:
    size_t size;
    jack_ringbuffer_read(ringBuffer, (char*)&size, sizeof(size_t));
    byteArray.resize(size);
    jack_ringbuffer_read(ringBuffer, byteArray.data(), size);
    // read the event from our byte array buffer:
    QDataStream stream(byteArray);
    event->read(stream);
    return event;
}

RingBufferEvent * RingBuffer::read()
{
    jack_nframes_t time;
    return read(time);
}
