#ifndef JACKTHREAD_H
#define JACKTHREAD_H

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

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "jackringbuffer.h"
#include "jackclient.h"

/**
  This is a utility class to be used together with a JackClient subclass.
  It simplifies deferred processing, i.e. putting those processing steps into
  a separate thread, which cannot be put into the Jack process() function, e.g.
  because they involve locking etc. and thus aren't real-time capable.

  The general idea is the following: this thread waits until wake() is called
  from outside (i.e., from the Jack process() function). It then calls processDeferred()
  to do any necessary processing. If done, it waits again. This is repeated until
  stop() is called from outside (i.e., from the Jack thread, usually from the deinit()
  function).

  For communication between this thread and the Jack process thread you should use
  JackRingBuffer.

  The following general procedure is recommended:
  <ol>
  <li>Always associate a JackThread object with a JackClient object.
  <li>Initialize all necessary ring buffers for communication between the Jack thread
  and this thread, and be sure that both objects have access to them. For each one-way
  communication use one ring buffer. E.g., if you need data to be communicated between threads
  in BOTH ways, use TWO ring buffers.
  <li>From JackClient's init() function, call JackThread's start() method. Also reset all ring buffers.
  <li>In JackClient's process() function, read all data from the JackThread->JackClient ring buffers,
  if there is any. Write data for deferred processing to the JackClient->JackThread ring buffers
  and call JackThread's wake() function.
  <li>In JackThread's processDeferred() function, read data from the JackClient->JackThread ring buffers
  and write data to the JackThread->JackClient ring buffers. You do not need to call any additional
  method after writing, as the Jack process() function will be called regularly to read from the
  ring buffers.
  <li>In JackClient's deinit() function, call JackThread's stop() method. After that you may want
  to reset the ring buffers.
  </ol>
  */

class JackThread : public QThread
{
    Q_OBJECT
public:
    explicit JackThread(JackClient *client, QObject *parent = 0);
    virtual ~JackThread();

    JackClient * getClient();

public slots:
    /**
      Sends a stop signal to the thread. When the thread leaves processDeferred()
      the thread is stopped.
      This function is blocking, i.e., it waits for the thread to finish
      processDeferred() (if it's currently running) before returning.
      */
    void stop();
    /**
      Wakes the thread to resume processing (i.e., call processDeferred()) or to
      end (if called by stop()).
      */
    void wake();

protected:
    /**
      Override this method to implement deferred processing in a separate thread.
      */
    virtual void processDeferred() = 0;

    void run();

private:
    JackClient *client;
    JackRingBuffer<bool> stopRingBuffer;
    QMutex mutex;
    QWaitCondition waitCondition;
};

#endif // JACKTHREAD_H
