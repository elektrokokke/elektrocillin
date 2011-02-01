#include "jackthread.h"

JackThread::JackThread(JackClient *client_, QObject *parent) :
    QThread(parent),
    client(client_),
    stopRingBuffer(1)
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
