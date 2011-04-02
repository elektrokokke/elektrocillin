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

#include "jackthread.h"

JackThread::JackThread(JackClient *client_, QObject *parent) :
    QThread(parent),
    client(client_),
    stopRingBuffer(1)
{
}

JackThread::~JackThread()
{
}

JackClient * JackThread::getClient()
{
    return client;
}

void JackThread::stop()
{
    // send the stop signal via the ring buffer:
    bool b = true;
    stopRingBuffer.write(&b, 1);
    // wake the thread:
    wake();
    // wait for the thread to finish:
    wait();
}
void JackThread::wake()

{
    waitCondition.wakeOne();
}

void JackThread::run()
{
    // lock the mutex (we need the lock for the wait condition):
    mutex.lock();
    // call processDeferred() unti the stop signal is received:
    bool stop = false;
    for (; !stop; ) {
        waitCondition.wait(&mutex);
        // test if we received the stop signal:
        if (stopRingBuffer.readSpace()) {
            stopRingBuffer.read(&stop, 1);
        }
        if (!stop) {
            // no stop signal received:
            processDeferred();
        }
    }
    // we received the stop signal
    // release the lock:
    mutex.unlock();
}
