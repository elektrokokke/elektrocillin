#ifndef METAJACKCONTEXT_H
#define METAJACKCONTEXT_H

#include "metajack/types.h"
#include "jackringbuffer.h"
#include <QWaitCondition>
#include <QMutex>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <string>
#include <set>
#include <map>
#include <utility>

/**
  This class implements a container client for JACK clients using the meta_jack
  library instead of the original JACK API.

  Each instance of this class represents a virtual JACK server where clients can
  register if they are linked to meta_jack. All these "virtual" clients will be
  hidden from the real JACK server. They can be connected just like normal JACK
  clients, and the whole graph will be represented in the real JACK server by one
  single client.

  The first implementation has one audio and MIDI input and one audio and MIDI output.

  Note: this class is to be used by meta_jack only, and not directly by clients
  which want to register at the virtual server!

  This class tries to achieve thread safety between the process thread and the
  other threads, but not between threads other than the process thread.
  Thread safety with the process thread is achieved by having two copies of the
  graph structure and communicating graph structure changes through a non-blocking
  ring buffer.
  */

class MetaJackClient;
class MetaJackPort;

struct MetaJackGraphEvent {
    enum {
        ACTIVATE_CLIENT,
        DEACTIVATE_CLIENT,
        REGISTER_PORT,
        UNREGISTER_PORT,
        CONNECT_PORTS,
        DISCONNECT_PORTS
    } type;
    MetaJackClient *client;
    MetaJackPort *source, *dest;
};

struct MetaJackContextMidiBufferHead {
    size_t bufferSize, midiEventCount, midiDataSize;
    jack_nframes_t lostMidiEvents;
};

class MetaJackContext
{
public:
    static MetaJackContext *instance;
    static MetaJackContext instance_;

    MetaJackContext(const std::string &name);
    ~MetaJackContext();

    bool isActive() const;

    // client- and port-related methods:
    MetaJackClient * openClient(const std::string &name, jack_options_t options);
    int connectPorts(const std::string &source_port, const std::string &destination_port);
    int disconnectPorts(const std::string &source_port, const std::string &destination_port);
    const char ** getPortsbyPattern(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags);
    MetaJackPort * getPortByName(const std::string &port_name) const;
    MetaJackPort * getPortById(jack_port_id_t port_id);

    int get_pid();
    pthread_t get_thread_id();
    bool is_realtime();

    // callback setting methods:
    int     set_thread_init_callback (MetaJackClient *client, JackThreadInitCallback thread_init_callback, void *arg);
    void    set_shutdown_callback (MetaJackClient *client, JackShutdownCallback shutdown_callback, void *arg);
    void    set_info_shutdown_callback (MetaJackClient *client, JackInfoShutdownCallback shutdown_callback, void *arg);
    int     set_freewheel_callback (MetaJackClient *client, JackFreewheelCallback freewheel_callback, void *arg);
    int     set_buffer_size_callback (MetaJackClient *client, JackBufferSizeCallback bufsize_callback, void *arg);
    int     set_sample_rate_callback (MetaJackClient *client, JackSampleRateCallback srate_callback, void *arg);
    int     set_client_registration_callback (MetaJackClient *, JackClientRegistrationCallback registration_callback, void *arg);
    int     set_port_registration_callback (MetaJackClient *, JackPortRegistrationCallback registration_callback, void *arg);
    int     set_port_connect_callback (MetaJackClient *, JackPortConnectCallback connect_callback, void *arg);
    int     set_port_rename_callback (MetaJackClient *, JackPortRenameCallback rename_callback, void *arg);
    int     set_graph_order_callback (MetaJackClient *, JackGraphOrderCallback graph_callback, void *);
    int     set_xrun_callback (MetaJackClient *, JackXRunCallback xrun_callback, void *arg);

    // server-related methods:
    int set_freewheel(int onoff);
    int set_buffer_size(jack_nframes_t nframes);
    jack_nframes_t get_sample_rate();
    jack_nframes_t get_buffer_size();
    float get_cpu_load();


    // time handling methods:
    jack_nframes_t  get_frames_since_cycle_start() const;
    jack_nframes_t  get_frame_time() const;
    jack_nframes_t  get_last_frame_time() const;
    jack_time_t     convert_frames_to_time(jack_nframes_t nframes) const;
    jack_nframes_t  convert_time_to_frames(jack_time_t time) const;
    static jack_time_t get_time();

    // MIDI-related methods:
    static jack_nframes_t      midi_get_event_count(void* port_buffer);
    static int                 midi_event_get(jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index);
    static void                midi_clear_buffer(void *port_buffer);
    static size_t              midi_max_event_size(void* port_buffer);
    static jack_midi_data_t*   midi_event_reserve(void *port_buffer, jack_nframes_t  time, size_t data_size);
    static int                 midi_event_write(void *port_buffer, jack_nframes_t time, const jack_midi_data_t *data, size_t data_size);
    static jack_nframes_t      midi_get_lost_event_count(void *port_buffer);

    jack_port_id_t createUniquePortId();

    // for all methods returning const char **:
    void free(void* ptr);

    // methods to signal changes in the process graph:
    void registeredClient(MetaJackClient *client);
    void unregisteredClient(MetaJackClient *client);
    void activatedClient(MetaJackClient *client);
    void deactivatedClient(MetaJackClient *client);
    void registeredPort(MetaJackPort *port);
    void unregisteredPort(MetaJackPort *port);
    void renamedPort(const std::string &oldFullName, const std::string &newFullName);
    void connectedPorts(MetaJackPort *a, MetaJackPort *b);
    void disconnectedPorts(MetaJackPort *a, MetaJackPort *b);

private:
    jack_client_t *wrapperClient;
    jack_port_t *wrapperAudioInputPort, *wrapperAudioOutputPort, *wrapperMidiInputPort, *wrapperMidiOutputPort;
    jack_port_id_t uniquePortId;
    std::set<MetaJackClient*> metaClients;
    std::map<std::string, MetaJackClient*> metaClientsByName;
    std::map<jack_port_id_t, MetaJackPort*> metaPortsById;
    std::map<std::string, MetaJackPort*> metaPortsByFullName;
    std::map<MetaJackClient*, std::pair<JackThreadInitCallback, void*> > threadInitCallbacks;
    std::map<MetaJackClient*, std::pair<JackShutdownCallback, void*> > shutdownCallbacks;
    std::map<MetaJackClient*, std::pair<JackInfoShutdownCallback, void*> > infoShutdownCallbacks;
    std::map<MetaJackClient*, std::pair<JackFreewheelCallback, void*> > freewheelCallbacks;
    std::map<MetaJackClient*, std::pair<JackBufferSizeCallback, void*> > bufferSizeCallbacks;
    std::map<MetaJackClient*, std::pair<JackSampleRateCallback, void*> > sampleRateCallbacks;
    std::map<MetaJackClient*, std::pair<JackClientRegistrationCallback, void*> > clientRegistrationCallbacks;
    std::map<MetaJackClient*, std::pair<JackPortRegistrationCallback, void*> > portRegistrationCallbacks;
    std::map<MetaJackClient*, std::pair<JackPortConnectCallback, void*> > portConnectCallbacks;
    std::map<MetaJackClient*, std::pair<JackPortRenameCallback, void*> > portRenameCallbacks;
    std::map<MetaJackClient*, std::pair<JackGraphOrderCallback, void*> > graphOrderCallbacks;
    std::map<MetaJackClient*, std::pair<JackXRunCallback, void*> > xRunCallbacks;
    std::set<MetaJackClient*> activeClients;
    JackRingBuffer<MetaJackGraphEvent> graphChangesRingBuffer;
    QWaitCondition waitCondition;
    QMutex waitMutex;

    // invoke callbacks:
    void invokeJackClientRegistrationCallbacks(const std::string &clientName, bool registered);
    void invokeJackPortRegistrationCallbacks(jack_port_id_t id, bool registered);
    void invokeJackPortRenameCallbacks(jack_port_id_t id, const std::string &oldName, const std::string &newName);
    void invokeJackPortConnectCallbacks(jack_port_id_t a, jack_port_id_t b, bool connected);
    // signal graph change:
    void sendGraphChangeEvent(const MetaJackGraphEvent &event);

    int process(jack_nframes_t nframes);
    static int process(jack_nframes_t nframes, void *arg);
};

class MetaJackClient {
public:
    MetaJackClient(MetaJackContext *context, const std::string &name, bool processThread);
    ~MetaJackClient();

    static size_t getNameSize();

    // JACK API methods:
    // to close a client, just delete it.
    const std::string & getName() const;
    bool activate();
    bool deactivate();
    bool setProcessCallback(JackProcessCallback process_callback, void *arg);
    MetaJackPort * registerPort(const std::string &port_name, const std::string &port_type, unsigned long flags, unsigned long buffer_size);
    bool hasPort(const MetaJackPort *port) const;

    MetaJackContext * getContext();
    const MetaJackContext * getContext() const;

    bool isActive() const;
    MetaJackClient * getTwin();
    void deleteTwin();

    void addPort(MetaJackPort *port);
    void removePort(MetaJackPort *port);

    bool residesInProcessThread() const;

    bool process(std::set<MetaJackClient*> &unprocessedClients, jack_nframes_t nframes);
private:
    void disconnect();

    MetaJackContext *context;
    std::string name;
    JackProcessCallback processCallback;
    void* processCallbackArgument;
    std::set<MetaJackPort*> ports;
    MetaJackClient *twin;
    bool processThread;
};

class MetaJackDummyInputClient : public MetaJackClient {
public:
    MetaJackDummyInputClient(MetaJackContext *context, jack_port_t *wrapperAudioInputPort, jack_port_t *wrapperMidiInputPort);

private:
    MetaJackPort *audioOutputPort, *midiOutputPort;
    jack_port_t *wrapperAudioInputPort, *wrapperMidiInputPort;
    static int process(jack_nframes_t nframes, void *arg);
};

class MetaJackDummyOutputClient : public MetaJackClient {
public:
    MetaJackDummyOutputClient(MetaJackContext *context, jack_port_t *wrapperAudioOutputPort, jack_port_t *wrapperMidiOutputPort);

private:
    MetaJackPort *audioInputPort, *midiInputPort;
    jack_port_t *wrapperAudioOutputPort, *wrapperMidiOutputPort;
    static int process(jack_nframes_t nframes, void *arg);
};

class MetaJackPort {
public:
    MetaJackPort(MetaJackClient *client, jack_port_id_t id, const std::string &short_name, const std::string &type, int flags);
    ~MetaJackPort();

    static size_t getNameSize();
    static size_t getTypeSize();

    const std::string & getShortName() const;
    bool setShortName(const std::string &port_name);
    const std::string & getFullName() const;
    const std::string & getType() const;
    jack_port_id_t getId() const;
    int getFlags() const;
    size_t getConnectionCount() const;
    bool isConnectedTo(MetaJackPort *port) const;
    bool isConnectedTo(const std::string &port_name) const;
    const char ** getConnections() const;

    void * getBuffer(jack_nframes_t nframes);

    bool isActive() const;

    void disconnect();
    void disconnect(MetaJackPort *port);
    void connect(MetaJackPort *port);
    bool isInput();


    MetaJackContext * getContext();
    const MetaJackContext * getContext() const;

    bool residesInProcessThread() const;

    MetaJackPort * getTwin();
    void deleteTwin();

    void clearBuffer();
    void mergeConnectedBuffers();

    bool process(std::set<MetaJackClient*> &unprocessedClients, jack_nframes_t nframes);
private:
    MetaJackClient *client;
    jack_port_id_t id;
    std::string short_name, full_name, type;
    int flags;
    size_t bufferSize;
    char *buffer;
    std::set<MetaJackPort*> connectedPorts;
    MetaJackPort *twin;
};

#endif // METAJACKCONTEXT_H
