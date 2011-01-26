#include "jackclientwithdeferredprocessing.h"

JackClientWithDeferredProcessing::JackClientWithDeferredProcessing(const QString &clientName, JackThread *deferTo) :
        JackClient(clientName),
        jackThread(deferTo)
{
}

JackThread * JackClientWithDeferredProcessing::getJackThread()
{
    return jackThread;
}

bool JackClientWithDeferredProcessing::init()
{
    jackThread->start();
    return true;
}

void JackClientWithDeferredProcessing::deinit()
{
    jackThread->stop();
}
