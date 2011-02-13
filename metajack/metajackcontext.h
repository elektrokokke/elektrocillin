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

class _meta_jack_client;
class _meta_jack_port;

struct MetaJackContextEvent {
    enum {
        ACTIVATE_CLIENT,
        DEACTIVATE_CLIENT,
        UNREGISTER_PORT,
        CONNECT_PORTS,
        DISCONNECT_PORTS
    } type;
    _meta_jack_client *client;
    _meta_jack_port *source, *dest;
};

struct MetaJackContextMidiBufferHead {
    size_t bufferSize, midiEventCount, midiDataSize;
};

class MetaJackContext
{
public:
    static MetaJackContext *instance;

    MetaJackContext(const std::string &name);
    ~MetaJackContext();

    // client-related methods:
    meta_jack_client_t * client_open(const std::string &name, jack_options_t options);
    int client_close(meta_jack_client_t *client);
    int client_get_name_size();
    const std::string & client_get_name(meta_jack_client_t *client) const;
    int client_activate(meta_jack_client_t *client);
    int client_deactivate(meta_jack_client_t *client);

    int get_pid();
    pthread_t get_thread_id();
    bool is_realtime();

    // callback setting methods:
    int     set_thread_init_callback (meta_jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg);
    void    set_shutdown_callback (meta_jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg);
    void    set_info_shutdown_callback (meta_jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg);
    int     set_process_callback (meta_jack_client_t *client, JackProcessCallback process_callback, void *arg);
    int     set_freewheel_callback (meta_jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg);
    int     set_buffer_size_callback (meta_jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg);
    int     set_sample_rate_callback (meta_jack_client_t *client, JackSampleRateCallback srate_callback, void *arg);
    int     set_client_registration_callback (meta_jack_client_t *, JackClientRegistrationCallback registration_callback, void *arg);
    int     set_port_registration_callback (meta_jack_client_t *, JackPortRegistrationCallback registration_callback, void *arg);
    int     set_port_connect_callback (meta_jack_client_t *, JackPortConnectCallback connect_callback, void *arg);
    int     set_port_rename_callback (meta_jack_client_t *, JackPortRenameCallback rename_callback, void *arg);
    int     set_graph_order_callback (meta_jack_client_t *, JackGraphOrderCallback graph_callback, void *);
    int     set_xrun_callback (meta_jack_client_t *, JackXRunCallback xrun_callback, void *arg);

    int set_freewheel(int onoff);
    int set_buffer_size(jack_nframes_t nframes);
    jack_nframes_t get_sample_rate();
    jack_nframes_t get_buffer_size();
    float get_cpu_load();

    // port-related methods:
    meta_jack_port_t * port_register(meta_jack_client_t *client, const std::string &port_name, const std::string &port_type, unsigned long flags, unsigned long buffer_size);
    int     port_unregister(meta_jack_port_t *port);
    void *  port_get_buffer(meta_jack_port_t *port, jack_nframes_t);
    const std::string & port_get_name(const meta_jack_port_t *port);
    const std::string & port_get_short_name(const meta_jack_port_t *port);
    int     port_get_flags(const meta_jack_port_t *port);
    const std::string & port_get_type(const meta_jack_port_t *port);
    jack_port_type_id_t port_get_type_id(const meta_jack_port_t *port);
    bool    port_is_mine (const meta_jack_client_t *client, const meta_jack_port_t *port);
    int     port_get_connection_count(const meta_jack_port_t *port);
    bool    port_is_connected_to(const meta_jack_port_t *port, const std::string &port_name);
    const char ** port_get_connections(const meta_jack_port_t *port);
    const char ** port_get_all_connections(const meta_jack_port_t *port);
    jack_nframes_t port_get_latency (meta_jack_port_t *port);
    jack_nframes_t port_get_total_latency(meta_jack_port_t *port);
    int     port_set_name(meta_jack_port_t *port, const std::string &port_name);
    int     port_set_alias(meta_jack_port_t *port, const std::string &alias);
    int     port_unset_alias(meta_jack_port_t *port, const std::string &alias);
    int     port_get_aliases(const meta_jack_port_t *port, char* const aliases[2]);
    int     port_connect(const std::string &source_port, const std::string &destination_port);
    int     port_disconnect(const std::string &source_port, const std::string &destination_port);
    int     port_disconnect(meta_jack_client_t *client, meta_jack_port_t *port);
    int     port_get_name_size();
    int     port_get_type_size();

    const char ** get_ports(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags);
    meta_jack_port_t * get_port_by_name(const std::string &port_name);
    meta_jack_port_t * get_port_by_id(jack_port_id_t port_id);

    // time handling methods:
    jack_nframes_t  get_frames_since_cycle_start();
    jack_nframes_t  get_frame_time();
    jack_nframes_t  get_last_frame_time();
    jack_time_t     convert_frames_to_time(jack_nframes_t nframes);
    jack_nframes_t  convert_time_to_frames(jack_time_t time);
    jack_time_t     get_time();

    jack_nframes_t      midi_get_event_count(void* port_buffer);
    int                 midi_event_get(jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index);
    void                midi_clear_buffer(void *port_buffer);
    size_t              midi_max_event_size(void* port_buffer);
    jack_midi_data_t*   midi_event_reserve(void *port_buffer, jack_nframes_t  time, size_t data_size);
    int                 midi_event_write(void *port_buffer, jack_nframes_t time, const jack_midi_data_t *data, size_t data_size);
    jack_nframes_t      midi_get_lost_event_count(void *port_buffer);

    // for all methods returning const char **:
    void free(void* ptr);

private:
    std::string name;
    jack_client_t *client;
    jack_port_t *audioIn, *audioOut, *midiIn, *midiOut;
    bool active;
    jack_port_id_t currentPortId;
    _meta_jack_client *dummyInputClient, *dummyOutputClient;
    std::set<meta_jack_client_t*> metaClients;
    std::map<std::string, meta_jack_client_t*> metaClientsByName;
    std::map<jack_port_id_t, meta_jack_port_t*> metaPortsById;
    std::map<std::string, meta_jack_port_t*> metaPortsByFullName;
    std::map<meta_jack_client_t*, std::pair<JackThreadInitCallback, void*> > threadInitCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackShutdownCallback, void*> > shutdownCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackInfoShutdownCallback, void*> > infoShutdownCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackFreewheelCallback, void*> > freewheelCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackBufferSizeCallback, void*> > bufferSizeCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackSampleRateCallback, void*> > sampleRateCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackClientRegistrationCallback, void*> > clientRegistrationCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackPortRegistrationCallback, void*> > portRegistrationCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackPortConnectCallback, void*> > portConnectCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackPortRenameCallback, void*> > portRenameCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackGraphOrderCallback, void*> > graphOrderCallbacks;
    std::map<meta_jack_client_t*, std::pair<JackXRunCallback, void*> > xRunCallbacks;

    std::set<meta_jack_client_t*> metaClientsForProcess;
    JackRingBuffer<MetaJackContextEvent> graphChangesRingBuffer;
    QWaitCondition waitCondition;
    QMutex waitMutex;

    int process(_meta_jack_client *metaClient, std::set<_meta_jack_client*> &unprocessedClients, jack_nframes_t nframes);
    int process(jack_nframes_t nframes);
    static int process(jack_nframes_t nframes, void *arg);
    static int processDummyInputClient(jack_nframes_t nframes, void *arg);
    static int processDummyOutputClient(jack_nframes_t nframes, void *arg);
};

class _meta_jack_client {
public:
    _meta_jack_client(MetaJackContext *context_, const std::string &name_);
    ~_meta_jack_client();
    void disconnect();

    _meta_jack_client * createTwin();
    _meta_jack_port * createPortTwin(_meta_jack_port *port);

    MetaJackContext *context;
    std::string name;
    JackProcessCallback processCallback;
    void* processCallbackArgument;
    std::set<_meta_jack_port*> ports;
    std::map<std::string, _meta_jack_port*> portsByShortName;
    _meta_jack_client *twin;
};

class _meta_jack_port {
public:
    _meta_jack_port(_meta_jack_client *client_, jack_port_id_t id_, const std::string &short_name_, const std::string &type_, int flags_);
    ~_meta_jack_port();
    std::string getFullName();
    void disconnect();
    void disconnect(_meta_jack_port *port);
    void connect(_meta_jack_port *port);
    bool isInput();

    _meta_jack_port * createTwin();

    void clearBuffer();
    void mergeBuffers(_meta_jack_port *destination_port);

    _meta_jack_client *client;
    jack_port_id_t id;
    std::string short_name, type, aliases[2], full_name;
    int flags;
    size_t bufferSize;
    char *buffer;
    std::set<_meta_jack_port*> connectedPorts;
    _meta_jack_port *twin;
};

#endif // METAJACKCONTEXT_H
