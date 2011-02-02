#ifndef JACKCLIENTWITHDEFERREDPROCESSING_H
#define JACKCLIENTWITHDEFERREDPROCESSING_H

#include "jackclient.h"
#include "jackthread.h"

/**
  This is the base class for Jack clients that do deferred (i.e., non real-time) processing
  in a separate thread (i.e., a JackThread instance).

  It is an abstract class, as it does not implement process().
  */

class JackClientWithDeferredProcessing : public JackClient
{
public:
    JackClientWithDeferredProcessing(const QString &clientName, JackThread *deferTo);

    JackThread * getJackThread();
    void wakeJackThread();

protected:
    /**
      This starts the associated JackThread.
      You must call this in your reimplementation, but only AFTER initializing any ring buffers
      the JackThread might use (you could of course also initialize ring buffers in the
      constructor instead).
      */
    virtual bool init();
    /**
      This stops the associated JackThread.
      You must call this in your reimplementation, but only BEFORE freeing any ring buffers
      the JackThread might use.
      */
    virtual void deinit();

private:
    JackThread *jackThread;
};

#endif // JACKCLIENTWITHDEFERREDPROCESSING_H
