#ifndef JACKCLIENT_H
#define JACKCLIENT_H

#include "metajack/jack.h"
#include <QStringList>

/**
  This is an abstract class which should simplify the creation of arbitrary Jack clients.
  The necessary methods to reimplement are:

  - init() and
  - process(jack_nframes_t nframes)

  First have a look at the already existing subclasses to see if your use-case is already
  covered by one of them. For clients with a fixed number of audio in- and outputs and no
  other ports you should use AudioProcessorClient. For clients with a MIDI input you
  should use MidiProcessorClient. For clients which have to receive signals from threads
  other than the process thread you can use the template class EventProcessorClient with your
  custom event type.
  */

class JackClient
{
public:
    /**
      @param clientName the name of the Jack client. The actual name may vary,
        as it may be changed by the Jack server if the given name is already taken
        by another client. Use getClientName() to get the actual name.
      */
    JackClient(const QString &clientName);

    /**
      @return If the client is active, this returns the actual client name as
        assign by the Jack server. If not, it returns the desired client name
        as given in the constructor.
      */
    const QString & getClientName() const;

    /**
      Activates the Jack client, i.e. starts the Jack process thread.
      The implementation first registers as a client to the Jack server,
      then registers the process callback and then calls init().
      If everything succeeded, the client is activated and calls to
      process(jack_nframes_t nframes) will start in a separate thread.

      Calling this method when the client is already active has no effect.

      This method should typically be called from the applications main thread.

      @return true if the client could be activated, false otherwise. Reasons
        for failure can be: the client could not be registered, the process
        callback could not be registered, the init() function returned false
        or the client could not be activated.
      */
    bool activate();
    /**
      Deactivates the Jack client. This implementation deactivates the client
      and then calls deinit(). I.e., when deinit() is called there will be no
      more calls to process(jack_nframes_t nframes).

      This method should typically be called from the applications main thread.
      */
    void close();

    bool isActive() const;
    /**
      Returns the current (estimated) Jack server time.
      From the Jack documentation (names are translated to this
      API when appropriate):

      This function is intended for use in other threads (not the process
      callback). The return value can be compared with the value of
      getLastFrameTime() to relate time in other threads to Jack time.
      */
    jack_nframes_t getEstimatedCurrentTime();
    /**
      Connects two given ports. Port names are constructed in the form
      "[clientname]:[portname]". Both ports have to be of the same type
      (e.g., two audio ports or two midi ports). You can only connect
      an output port to an input port.

      @param sourcePortName the name of the source port to be connected.
        The port has to be an output port
      @param destPortName the name of the destination port to be connected.
        The port has to be an input port
      @return true if the connection could be made, false otherwise
      */
    bool connectPorts(const QString &sourcePortName, const QString &destPortName);
    /**
      Disconnects two given ports. Port names are constructed in the form
      "[clientname]:[portname]". Both ports have to be of the same type
      (e.g., two audio ports or two midi ports).

      @param sourcePortName the name of the source port to be connected.
        The port has to be an output port
      @param destPortName the name of the destination port to be connected.
        The port has to be an input port
      @return true if the ports could be disconnected, false otherwise
      */
    bool disconnectPorts(const QString &sourcePortName, const QString &destPortName);

    QString getShortPortname(jack_port_t *port);
    QString getFullPortname(jack_port_t *port);
    QStringList getClientPorts(const QString &clientName, unsigned long flags = 0);
    QStringList getAllPorts(unsigned long flags = 0);
    QStringList getAllConnectedPorts(const QString &fullPortName);
    QStringList getAllConnections();
    void restoreConnections(const QStringList &connections);

    static QString getFullPortName(const QString &clientName, const QString &shortPortName);
    static int getMaximumPortNameLength();

protected:
    /**
      This is called before the jack client is activated.
      You should register the input and output ports here.

      @return true if the initialization succeeded, false otherwise
      */
    virtual bool init() = 0;
    /**
      This is called after the jack client is closed.
      The default implementation does nothing.
      */
    virtual void deinit();
    /**
      Implement this method to do the actual processing.
      You might first want to get the port buffers and then
      read from and/or write to them here.

      This method is called in the jack process thread.
      You must not use any locks, e.g., allocate memory, access
      a disk etc., i.e. nothing which is not real-time safe.
      */
    virtual bool process(jack_nframes_t nframes) = 0;
    /**
      This function must not be called from outside the process
      thread!
      From the Jack documentation (names are translated to this
      API when appropriate):

      @return the precise time at the start of the current process cycle.
      This function may only be used from the process callback, and can
      be used to interpret timestamps generated by getEstimatedCurrentTime() in
      other threads with respect to the current process cycle.

      This is the only jack time function that returns exact time:
      when used during the process callback it always returns the same
      value (until the next process callback, where it will return
      that value + nframes, etc).  The return value is guaranteed to be
      monotonic and linear in this fashion unless an xrun occurs.
      If an xrun occurs, clients must check this value again, as time
      may have advanced in a non-linear way (e.g. cycles may have been skipped).
      */
    jack_nframes_t getLastFrameTime();
    /**
      This method retrieves the sample rate of the Jack server. You will
      find the sample rate useful for things like oscillators, filters etc.

      This method may only be called when the client is registered at the
      server, i.e. in or after the init() method call and before the client
      is closed.

      @return the sample rate of the Jack server in Hertz
      */
    jack_nframes_t getSampleRate() const;
    /**
      Creates an audio port with a given name.

      @param the name of the audio port
      @param flags specifies the port properties. Possible values are:
        JackPortIsInput - the port should be an input port
        JackPortIsOutput - the port should be an output port
      @return a pointer to the newly create port if successful, 0 otherwise
      */
    jack_port_t * registerAudioPort(const QString &name, unsigned long flags);
    /**
      Creates a MIDI port with a given name.

      @param the name of the MIDI port
      @param flags specifies the port properties. Possible values are:
        JackPortIsInput - the port should be an input port
        JackPortIsOutput - the port should be an output port
      @return a pointer to the newly create port if successful, 0 otherwise
      */
    jack_port_t * registerMidiPort(const QString &name, unsigned long flags);

private:
    QString requestedName, actualName;
    jack_client_t *client;

    static int process(jack_nframes_t nframes, void *arg);

    friend class JackNullClient;
};

#endif // JACKCLIENT_H
