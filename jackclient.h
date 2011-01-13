#ifndef JACKCLIENT_H
#define JACKCLIENT_H

#include <jack/jack.h>

#include <QString>
#include <QList>

class JackClient
{
public:
    JackClient(const QString &clientName);
    virtual ~JackClient();

    bool activate();

protected:
    virtual bool setup() = 0;
    virtual bool process(jack_nframes_t nframes) = 0;

    jack_port_t * registerAudioPort(const QString &name, unsigned long flags);
    jack_port_t * registerMidiPort(const QString &name, unsigned long flags);
    jack_nframes_t getSampleRate() const;

private:
    QString name;
    jack_client_t *client;

    static int process(jack_nframes_t nframes, void *arg);
};

#endif // JACKCLIENT_H
