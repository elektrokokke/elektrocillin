#ifndef JACKRINGBUFFER_H
#define JACKRINGBUFFER_H

#include <QObject>
#include <QDataStream>
#include <QByteArray>
#include <jack/ringbuffer.h>
#include <jack/types.h>

/**
  This is a C++ abstraction over jack_ringbuffer_t.
  It is a template class that allows reading and writing of structs of a given type T.
  T has to be a data type that can be copied just by copying the memory that its objects
  use (with size equal to sizeof(T)).
  E.g., classes with constructor and/or destructor generally don't qualify.
  Simple structs generally do.
 */

template<class T> class JackRingBuffer
{
public:
    JackRingBuffer(size_t size)
    {
        ringBuffer = jack_ringbuffer_create(size * sizeof(T));
    }
    ~JackRingBuffer()
    {
        jack_ringbuffer_free(ringBuffer);
    }

    /**
      @return The number of elements of type T that can be read from the ring buffer.
      */
    size_t readSpace() const
    {
        return jack_ringbuffer_read_space(ringBuffer) / sizeof(T);
    }
    /**
      @return The number of elements of type T that can be written to the ring buffer.
      */
    size_t writeSpace() const
    {
        return jack_ringbuffer_write_space(ringBuffer) / sizeof(T);
    }
    /**
      Reads a number of entries from the ring buffer.
      Calling this method does not advance the read pointer, i.e.,
      calling this multiple times successively always returns the same.
      Note: this method involves copying the data from the ring buffer
      to the given buffer, thus do not use it more often than necessary.
      @param buffer pointer to a buffer of type T where data will be copied to.
      It has to be large enough for at least n elements of type T.
      @param n the number of elements to be read from the ring buffer.
      */
    void peek(T *buffer, size_t n)
    {
        jack_ringbuffer_peek(ringBuffer, (char*)buffer, n * sizeof(T));
    }
    /**
      Takes a look at the next element in the ring buffer.
      This is a shortcut for peek(T *buffer, size_t n) with n=1.
      The compiler can optimize this to not copy from the ring buffer
      twice, i.e., it will directly copy the head of the ring buffer
      to the caller. Note that it will still be copied once, thus
      as with peek(T *buffer, size_t n) do not use it more often than
      necessary.
      @return the first entry in the ring buffer.
      */
    T peek()
    {
        T element;
        jack_ringbuffer_peek(ringBuffer, (char*)&element, sizeof(T));
        return element;
    }

    /**
      Reads a number of entries from the ring buffer.
      This is similar to peek(), but it advances the read pointer after
      reading.
      @param buffer pointer to a buffer of type T where data will be copied to.
        has to be large enough for at least n elements of type T.
      @param n the number of elements to be copied to the given buffer.
      */
    void read(T *buffer, size_t n)
    {
        jack_ringbuffer_read(ringBuffer, (char*)buffer, n * sizeof(T));
    }
    /**
      Reads one element from the ring buffer.
      This is a shortcut for read(T *buffer, size_t n) with n=1.
      The compiler can optimize this to not copy from the ring buffer
      twice, i.e., it will directly copy the first element in the ring buffer
      to the caller. After reading the read pointer will be advanced by one step.
      @return the first entry in the ring buffer.
      */
    T read()
    {
        T element;
        jack_ringbuffer_read(ringBuffer, (char*)&element, sizeof(T));
        return element;
    }

    /**
      Advances the read pointer of the ring buffer without reading from it.
      Use this, e.g., if you have used peek before.
      @param n the number of elements by which the read pointer should be advanced.
      */
    void readAdvance(size_t n)
    {
        jack_ringbuffer_read_advance(ringBuffer, n * sizeof(T));
    }

    /**
      Writes the given elements to the ring buffer and advances the write
      pointer.
      @param buffer pointer to a buffer of type T where data will be copied from.
      It has to contain at least n elements of type T.
      @param n the number of elements to be written to the ring buffer.
      */
    void write(const T *buffer, size_t n)
    {
        jack_ringbuffer_write(ringBuffer, (const char*)buffer, n * sizeof(T));
    }
    /**
      Writes one element to the ring buffer.
      This is a shortcut for write(const T *buffer, size_t n) with n=1.
      After writing the read pointer will be advanced by one step.
      @param element the element to write to the ring buffer.
      */
    void write(const T &element)
    {
        jack_ringbuffer_write(ringBuffer, (const char*)&element, sizeof(T));
    }

    /**
      Advances the write pointer of the ring buffer without writing something to it.
      @param n the number of elements by which the write pointer should be advanced.
      */
    void writeAdvance(size_t n)
    {
        jack_ringbuffer_write_advance(ringBuffer, n * sizeof(T));
    }
    /**
      Resets the ring buffer. Note: this method is NOT thread-safe.
      */
    void reset()
    {
        jack_ringbuffer_reset(ringBuffer);
    }

private:
    jack_ringbuffer_t * ringBuffer;
};

class RingBufferEvent {
public:
    virtual ~RingBufferEvent() {}
};

class RingBuffer
{
public:
    RingBuffer(size_t ringBufferSize);
    ~RingBuffer();

    bool hasEvents();
    jack_nframes_t peekEventTime();
    bool sendEvent(const RingBufferEvent *event, jack_nframes_t time);
    RingBufferEvent * readEvent(jack_nframes_t &time);
    RingBufferEvent * readEvent();
    void returnEvent(RingBufferEvent *event);
private:
    jack_ringbuffer_t *ringBuffer, *ringBufferReturn;
};

#endif // JACKRINGBUFFER_H
