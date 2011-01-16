#ifndef JACKCLIENT_H
#define JACKCLIENT_H

#include <jack/jack.h>

#include <QString>
#include <QList>

class JackClient
{
public:
    JackClient(const QString &clientName);

    bool activate();
    void close();

    bool isActive() const;

protected:
    /**
      This is called before the jack client is activated.
      You should register the input and output ports here.
      */
    virtual bool setup() = 0;
    /**
      This is called in the jack process thread.
      It must not use any locks, e.g., allocate memory, access
      a disk etc.
      */
    virtual bool process(jack_nframes_t nframes) = 0;
    /**
      This function must not be called from outside of process()!
      */
    jack_nframes_t getLastFrameTime();
    /**
      This function may be called from anyhwere.
      */
    jack_nframes_t getEstimatedCurrentTime();

    jack_port_t * registerAudioPort(const QString &name, unsigned long flags);
    jack_port_t * registerMidiPort(const QString &name, unsigned long flags);
    jack_nframes_t getSampleRate() const;

private:
    QString name;
    jack_client_t *client;

    static int process(jack_nframes_t nframes, void *arg);
};

#endif // JACKCLIENT_H
