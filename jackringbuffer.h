#ifndef JACKRINGBUFFER_H
#define JACKRINGBUFFER_H

#include <QObject>
#include <QDataStream>
#include <QByteArray>
#include <QSet>
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

/**
  This class enables lock-free communication between two threads, one
  "sender" thread and one "receiver" thread.

  The communication is based on "events". Any event that derives from
  RingBufferEvent can be sent through an object of this class. One
  instance only enables communication into one direction, thus you'll
  need two instances of this class to have a two-way communcation
  between two threads.

  Using this class basically follows these steps:

  0.) The receiver thread periodically calls hasEvents()
  1.) The sender thread calls sendEvent()
  2.) The receiver is not actively notified that a new event has been
      sent. Instead, its next call to hasEvents() returns true
  3.) The receiver calls readEvent() to get a pointer to the new event
  4.) The receiver does something using the event's data
  5.) When finished, the receiver calls returnEvent()
  6.) During the next call to sendEvent() (or when the ring buffer is
      is deleted), the returned event is deleted

  Typically the sender thread creates the event objects and deletes them
  if they have been processed. To make sure that this is so, you should
  call sendEvent() only from the sender thread, and also delete the
  ring buffer from the sending thread.

  Note: deleting an object of this class is not thread-safe. Make sure
  that noone except the deleting thread is accessing the ring buffer
  or any events which have been put into the ring buffer and which have
  not yet been returned, before you delete it!

  Note: It is probably not a good idea to use this class for
  communication between a Jack process thread (as sender) and another
  thread, because there is memory allocation (object creation) and
  deletion involved, which would happen in the Jack process thread.
  */
class RingBuffer
{
public:
    /**
      Initializes the ring buffer with the given size.

      @param ringBufferSize specifies the maximum number of events
        that can be in the ring buffer at the same time. I.e., the maximum
        number of events that can be sent via sendEvent() without any of
        them being retrieved via readEvent()
      */
    RingBuffer(size_t ringBufferSize);
    /**
      Deletes all events that not yet been deleted yet (this can happen for
      example when sendEvent() is not called anymore after events have been
      returned).

      Note: deleting an object of this class is not thread-safe. Make sure
      that noone except the deleting thread is accessing the ring buffer
      or any events which have been put into the ring buffer and which have
      not yet been returned, before you delete it!
      */
    ~RingBuffer();

    /**
      Tests if any events have been sent by the sender thread.
      This method is meant to be called by the receiver, but it can also
      safely be called by any other thread.

      @return true, iff any events are in the ring buffer
      */
    bool hasEvents();
    /**
      Allows to look at the event time of the first event in the ring buffer.
      Call hasEvents() before to make sure that there really is an event in
      the buffer.

      Calling this does not remove the first event from the ring buffer.

      This method is meant to be called by the receiver, but it can also
      safely be called by any other thread.

      @return the time of the first event in the buffer, as given to
        sendEvent() when the event was sent
      */
    jack_nframes_t peekEventTime();
    /**
      Puts an event into the ring buffer. This method is meant to be called
      by the sender thread.

      The ownership of the given event is transferred to the ring buffer. It
      is NOT safe to access the event after calling this method. The given
      event will be deleted by the ring buffer after it has been processed
      by the receiver.

      @param event a pointer to the event which should be put into the ring
        buffer. Ownership of the event is transferred to the ring buffer, and
        it is NOT safe to access it after calling this method
      @param time a time value, e.g., as given by Jack, which allows the event
        to be associated with a given time point. Note that this class does
        not take responsibility of sorting the given events temporally. The
        given value is not interpreted in any way. Its meaning does depend
        solely on what the sender and receiver define and agree upon
      */
    bool sendEvent(RingBufferEvent *event, jack_nframes_t time);
    /**
      Reads and returns an event from the ring buffer. Call hasEvents()
      before to make sure that there really is an event in the buffer.

      This method is meant to be called by the receiver.
      The returned event will be removed from the ring buffer.
      Call returnEvent() after you have processed the event to make sure
      that it is deleted properly.

      @param time a reference to where the event's time will be stored by
        this method
      @return a pointer to the first event in the buffer
      */
    RingBufferEvent * readEvent(jack_nframes_t &time);
    /**
      This is an overloaded function which can be used when the time
      associated with an event is not relevant to the receiver.

      See readEvent(jack_nframes_t &) for further information.

      @return a pointer to the first event in the buffer
      */
    RingBufferEvent * readEvent();
    /**
      This method is meant to be called by the receiver after
      it has processed an event read from the buffer and doesn't
      need access to it anymore.

      The given event will be deleted during the next call to sendEvent()
      (i.e., in the sender's thread) or when the ring buffer is deleted,
      whichever comes first.

      @param event a pointer to the event which should be deleted
      */
    void returnEvent(RingBufferEvent *event);
private:
    jack_ringbuffer_t *ringBuffer, *ringBufferReturn;
    QSet<RingBufferEvent*> undeletedEvents;
};

#endif // JACKRINGBUFFER_H
