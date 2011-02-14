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

struct MetaJackContextEvent {
    enum {
        ACTIVATE_CLIENT,
        DEACTIVATE_CLIENT,
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

    MetaJackContext(const std::string &name);
    ~MetaJackContext();

    // client-related methods:
    MetaJackClient * client_open(const std::string &name, jack_options_t options);
    void addClient(MetaJackClient *client);
    void removeClient(MetaJackClient *client);
    void addPort(MetaJackPort *port);
    void removePort(MetaJackPort *port);

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

    int set_freewheel(int onoff);
    int set_buffer_size(jack_nframes_t nframes);
    jack_nframes_t get_sample_rate();
    jack_nframes_t get_buffer_size();
    float get_cpu_load();

    // port-related methods:
    void *  port_get_buffer(MetaJackPort *port, jack_nframes_t);
    const std::string & port_get_name(const MetaJackPort *port);
    const std::string & port_get_short_name(const MetaJackPort *port);
    int     port_get_flags(const MetaJackPort *port);
    const std::string & port_get_type(const MetaJackPort *port);
    const char ** port_get_all_connections(const MetaJackPort *port);
    int     port_connect(const std::string &source_port, const std::string &destination_port);
    int     port_disconnect(const std::string &source_port, const std::string &destination_port);

    const char ** get_ports(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags);
    MetaJackPort * get_port_by_name(const std::string &port_name) const;
    MetaJackPort * get_port_by_id(jack_port_id_t port_id);

    // time handling methods:
    jack_nframes_t  get_frames_since_cycle_start() const;
    jack_nframes_t  get_frame_time() const;
    jack_nframes_t  get_last_frame_time() const;
    jack_time_t     convert_frames_to_time(jack_nframes_t nframes) const;
    jack_nframes_t  convert_time_to_frames(jack_time_t time) const;
    jack_time_t     get_time();

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

    // invoke callbacks:
    void invokeJackClientRegistrationCallbacks(const std::string &clientName, bool registered);
    void invokeJackPortRegistrationCallbacks(jack_port_id_t id, bool registered);
    void invokeJackPortRenameCallbacks(jack_port_id_t id, const std::string &oldName, const std::string &newName);
    void invokeJackPortConnectCallbacks(jack_port_id_t a, jack_port_id_t b, bool connected);

    // signal graph change:
    void sendGraphChangeEvent(const MetaJackContextEvent &event);

private:
    std::string name;
    jack_client_t *jackClient;
    jack_port_t *audioIn, *audioOut, *midiIn, *midiOut;
    bool active;
    jack_port_id_t currentPortId;
    MetaJackClient *dummyInputClient, *dummyOutputClient;
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

    std::set<MetaJackClient*> metaClientsForProcess;
    JackRingBuffer<MetaJackContextEvent> graphChangesRingBuffer;
    QWaitCondition waitCondition;
    QMutex waitMutex;

    int process(jack_nframes_t nframes);
    static int process(jack_nframes_t nframes, void *arg);
    static int processDummyInputClient(jack_nframes_t nframes, void *arg);
    static int processDummyOutputClient(jack_nframes_t nframes, void *arg);
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
