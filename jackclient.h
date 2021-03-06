#ifndef JACKCLIENT_H
#define JACKCLIENT_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "metajack/metajack.h"
#include "metajack/metajackclientserializer.h"
#include <QStringList>
#include <QMap>
#include <QRectF>
#include <QAction>
#include <QFont>

class QGraphicsItem;
class GraphicsClientItem;
class GraphicsClientItemsClient;

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

class JackClientFactory;

class JackClient : public QObject
{
    Q_OBJECT
public:
    /**
      @param clientName the name of the Jack client. The actual name may vary,
        as it may be changed by the Jack server if the given name is already taken
        by another client. Use getClientName() to get the actual name.
      */
    JackClient(const QString &clientName);
    virtual ~JackClient();

    virtual JackClientFactory * getFactory();
    /**
      Reimplement this to save the client's state to the given stream.

      Make sure that what is saved here can be loaded again by the
      corresponding method loadState(QDataStream&).

      The default implementation does nothing.
      */
    virtual void saveState(QDataStream &stream);
    /**
      Reimplement this to load the client's state from the given stream.

      Make sure that what is save by the corresponding method
      saveState(QDataStream&) can be loaded again here.

      The default implementation does nothing.
      */
    virtual void loadState(QDataStream &stream);

    /**
      Switch the use of the process callback on or off.
      This method only has an effect if called before
      activate().
      */
    void setCallProcess(bool processCallback);
    /**
      Switch the port registration and connection signals
      on or off. This method only has an effect if called
      before activate().
      */
    void setEmitPortSignals(bool portSignals);
    /**
      Switch the client registration signals on or off.
      This method only has an effect if called before
      activate().
      */
    void setEmitClientSignals(bool clientSignals);

    jack_client_t * getClient();

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

    /**
      @param fullPortName the full port name of the port whose type should be returned.
        This can be any port of any client
      @return the port type of the given port, e.g. "JACK_DEFAULT_AUDIO_TYPE"
        or "JACK_DEFAULT_MIDI_TYPE"
      */
    QString getPortType(const QString &fullPortName);
    /**
      @param fullPortName the full port name of the port whose flags should be returned.
        This can be any port of any client
      @return the flags of the given port, most notably including JackPortIsInput or
        JackPortIsOutput
      */
    int getPortFlags(const QString &fullPortName);

    /**
      Returns a list of ports that belong to this client and that match the given criteria.

      @param typeNamePattern a regular expression describing the type of the requested ports, e.g.,
        "JACK_DEFAULT_AUDIO_TYPE" will return all audio ports. A value of zero means all types are valid
      @param flags the flags of the requested ports. Only ports who have all the given flags set will be
        returned
      */
    QStringList getMyPorts(const char *typeNamePattern = 0, unsigned long flags = 0);
    /**
      Returns a list of ports that match the given criteria. This includes ports of other clients
      that match the given full port name regular expression.

      @param fullPortNamePattern a regular expression describing the full port names of the requested
        ports. E.g., "clientname:*" will match all ports belonging to the client with name "clientname";
        "*:portname" will match ports with the short name "portname" belonging to any client
      @param typeNamePattern a regular expression describing the type of the requested ports. E.g.,
        "JACK_DEFAULT_AUDIO_TYPE" will return all audio ports. A value of zero means all types are valid
      @param flags the flags of the requested ports. Only ports who have all the given flags set will be
        returned
      */
    QStringList getPorts(const char *fullPortNamePattern = 0, const char *typeNamePattern = 0, unsigned long flags = 0);

    QStringList getConnectedPorts(const QString &fullPortName);
    QStringList getConnections();
    void restoreConnections(const QStringList &connections);

    QStringList getClients();

    QString getPortNameById(jack_port_id_t id);

    void setClientItemPosition(QPointF pos);
    void setClientItemVisible(bool visible);
    virtual GraphicsClientItem * createClientItem(GraphicsClientItemsClient *clientItemsClient, int clientStyle, int audioPortStyle, int midiPortStyle, QFont font);
    /**
      Returns a QGraphicsItem object that serves as the graphical user interface
      for this client. The default implementation returns 0, which means that the
      client has no GUI. Subclasses may reimplement this to provide their own
      GUI.
      */
    virtual QGraphicsItem * createGraphicsItem();

    static QString getFullPortName(const QString &clientName, const QString &shortPortName);
    static int getMaximumPortNameLength();

public slots:
    void play();
    void stop();
    void rewind();

signals:
    void portConnected(QString sourcePortName, QString destPortName);
    void portDisconnected(QString sourcePortName, QString destPortName);
    void portRegistered(QString fullPortname, QString type, int flags);
    void portUnregistered(QString fullPortname, QString type, int flags);
    void clientRegistered(QString clientName);
    void clientUnregistered(QString clientName);

protected:
    /**
      This is called before the jack client is activated.
      You should register the input and output ports here.

      The default implementation does nothing and returns true.

      @return true if the initialization succeeded, false otherwise
      */
    virtual bool init();
    /**
      This is called after the jack client is closed.
      The default implementation does nothing.
      */
    virtual void deinit();
    /**
      Reimplement this method to do the actual processing.
      You might first want to get the port buffers and then
      read from and/or write to them here.

      The default method does nothing and returns true.

      This method is called in the jack process thread.
      You must not use any locks, e.g., allocate memory, access
      a disk etc., i.e. nothing which is not real-time safe.

      This method will never be called if you switched off
      processing by calling setUseProcessCallback(false).
      */
    virtual bool process(jack_nframes_t nframes);
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
    bool processCallback, portCallbacks, clientCallback;
    QPointF clientItemPosition;
    bool clientItemVisible;

    static int process(jack_nframes_t nframes, void *arg);
    static void portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void *arg);
    static void portRegistrationCallback(jack_port_id_t id, int registered, void *arg);
    static void clientRegistrationCallback(const char *name, int registered, void *arg);
};

class JackClientSerializer : public MetaJackClientSerializer
{
public:
    static JackClientSerializer * getInstance();

    void registerClient(jack_client_t *client, JackClient *jackClient);
    void unregisterClient(jack_client_t *client);
    JackClient * getClient(jack_client_t *client);

    JackClient * createClient(const QString &factoryName, const QString &clientName);
    void save(jack_client_t *client, QDataStream &stream);
    jack_client_t * load(const QString &clientName, QDataStream &stream);

    void registerFactory(JackClientFactory *factory);
    JackClientFactory * getFactoryByName(const QString &name);
    QList<JackClientFactory*> getFactories();
private:
    QMap<QString, JackClientFactory*> factories;
    QMap<jack_client_t*, JackClient*> mapClientHandlesToJackClients;

    static JackClientSerializer *serializer;

    JackClientSerializer();
};

class JackClientFactory
{
public:
    virtual QString getName() = 0;
    virtual JackClient * createClient(const QString &clientName) = 0;
};

#endif // JACKCLIENT_H
