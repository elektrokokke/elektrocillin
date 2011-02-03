#ifndef JACKUTILITYCLIENT_H
#define JACKUTILITYCLIENT_H

#include "jackclient.h"
#include <QStringList>

class JackNullClient : public JackClient
{
public:
    JackNullClient();
    ~JackNullClient();

    QStringList getPorts(unsigned long flags = 0);
    QStringList getPortConnections(const QString &portName);
    QStringList getAllConnections();
    void restoreConnections(const QStringList &connections);

protected:
    bool init();
    bool process(jack_nframes_t nframes);
};

#endif // JACKUTILITYCLIENT_H
