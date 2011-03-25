#ifndef METAJACKCONTEXTNEW_H
#define METAJACKCONTEXTNEW_H

#include "jackcontext.h"
#include "midiport.h"
#include "metajackclient.h"
#include "metajackport.h"
#include "callbackhandlers.h"
#include "jackringbuffer.h"
#include <map>
#include <QWaitCondition>
#include <QMutex>

class MetaJackContext : public JackContext
{
public:
    struct MetaJackContextMidiBufferHead {
        enum { MAGIC = 0x42424242 };
        uint32_t magic;
        size_t bufferSize, midiEventCount, midiDataSize;
        jack_nframes_t lostMidiEvents;
    };

    MetaJackContext(JackContext *wrapperInterface, const std::string &name, unsigned int oversampling = 1);
    virtual ~MetaJackContext();

    jack_port_t * createWrapperPort(const std::string &shortName, const std::string &type, unsigned long flags);

    jack_client_t * getWrapperClient();
    JackContext * getWrapperInterface();
    unsigned int getOversampling() const;

    bool isActive() const;

    jack_port_id_t createUniquePortId();

    // create and delete clients:
    MetaJackClient * openClient(const std::string &name, jack_options_t options);
    // methods to change the process graph
    // each of the following methods has a corresponding method which is called from the process thread
    bool closeClient(MetaJackClient *client);
    bool setProcessCallback(MetaJackClient *client, JackProcessCallback processCallback, void *processCallbackArgument);
    bool activateClient(MetaJackClient *client);
    bool deactivateClient(MetaJackClient *client);
    MetaJackPort * registerPort(MetaJackClient *client, const std::string & shortName, const std::string &type, unsigned long flags, unsigned long buffer_size);
    bool unregisterPort(MetaJackPort *port);
    bool renamePort(MetaJackPort *port, const std::string &shortName);
    bool connectPorts(const std::string &source_port, const std::string &destination_port);
    bool disconnectPorts(const std::string &source_port, const std::string &destination_port);

    // client- and port-related methods:
    const char ** getPortsByPattern(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags);
    MetaJackPort * getPortByName(const std::string &name) const;
    MetaJackPort * getPortById(jack_port_id_t id);
    void * getPortBuffer(MetaJackPort *port, jack_nframes_t nframes);

    // time handling methods:
    jack_nframes_t  get_frames_since_cycle_start() const;
    jack_nframes_t  get_frame_time() const;
    jack_nframes_t  get_last_frame_time() const;
    jack_time_t     convert_frames_to_time(jack_nframes_t nframes) const;
    jack_nframes_t  convert_time_to_frames(jack_time_t time) const;

    // MIDI-related methods:
    static void midi_init_buffer(void *port_buffer, size_t bufferSizeInBytes);
    static jack_nframes_t midi_get_event_count(void* port_buffer);
    static int midi_event_get(jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index);
    static void midi_clear_buffer(void *port_buffer);
    static size_t midi_max_event_size(void* port_buffer);
    static jack_midi_data_t * midi_event_reserve(void *port_buffer, jack_nframes_t time, size_t data_size);
    static int midi_event_write(void *port_buffer, jack_nframes_t time, const jack_midi_data_t *data, size_t data_size);
    static jack_nframes_t midi_get_lost_event_count(void *port_buffer);
    static bool compare_midi_events(const jack_midi_event_t &event1, const jack_midi_event_t &event2);

    // callback handlers for the internal clients:
    JackThreadInitCallbackHandler threadInitCallbackHandler;
    JackShutdownCallbackHandler shutdownCallbackHandler;
    JackInfoShutdownCallbackHandler infoShutdownCallbackHandler;
    JackFreewheelCallbackHandler freewheelCallbackHandler;
    JackBufferSizeCallbackHandler bufferSizeCallbackHandler;
    JackSampleRateCallbackHandler sampleRateCallbackHandler;
    JackClientRegistrationCallbackHandler clientRegistrationCallbackHandler;
    JackPortRegistrationCallbackHandler portRegistrationCallbackHandler;
    JackPortConnectCallbackHandler portConnectCallbackHandler;
    JackPortRenameCallbackHandler portRenameCallbackHandler;
    JackGraphOrderCallbackHandler graphOrderCallbackHandler;
    JackXRunCallbackHandler xRunCallbackHandler;
private:
    static MetaJackContext instance;

    class MetaJackGraphEvent {
    public:
        enum {
            CLOSE_CLIENT,
            SET_PROCESS_CALLBACK,
            ACTIVATE_CLIENT,
            DEACTIVATE_CLIENT,
            REGISTER_PORT,
            UNREGISTER_PORT,
            CONNECT_PORTS,
            DISCONNECT_PORTS,
            RENAME_PORT
        } type;
        MetaJackClientProcess *client;
        MetaJackPortProcess *port, *connectedPort;
        MetaJackPort *nonProcessPort;
        JackProcessCallback processCallback;
        void * processCallbackArgument;
        std::string shortName;
    };

    JackContext *wrapperInterface;
    jack_client_t *wrapperClient;
    std::string wrapperClientName;
    jack_nframes_t bufferSize;
    jack_port_id_t uniquePortId;
    std::map<std::string, MetaJackClient*> clients;
    std::map<std::string, MetaJackPort*> portsByName;
    std::map<jack_port_id_t, MetaJackPort*> portsById;
    std::map<MetaJackPort*,MetaJackPortProcess*> processPorts;
    std::set<MetaJackClientProcess*> activeClients;
    JackRingBuffer<MetaJackGraphEvent> graphChangesRingBuffer;
    QWaitCondition waitCondition;
    QMutex waitMutex;
    bool shutdown;
    unsigned int oversampling;

    void closeClient(MetaJackClientProcess *client);
    void setProcessCallback(MetaJackClientProcess *client, JackProcessCallback processCallback, void *processCallbackArgument);
    void activateClient(MetaJackClientProcess *client);
    void deactivateClient(MetaJackClientProcess *client);
    void registerPort(MetaJackClientProcess *client, MetaJackPortProcess *port, MetaJackPort *nonProcessPort);
    void unregisterPort(MetaJackPortProcess *port, MetaJackPort *nonProcessPort);
    void renamePort(MetaJackPortProcess *port, const std::string &shortName);
    void connectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest);
    void disconnectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest);

    // signal graph change:
    void sendGraphChangeEvent(const MetaJackGraphEvent &event);

    int process(jack_nframes_t nframes);
    static int process(jack_nframes_t nframes, void *arg);
    static void infoShutdownCallback(jack_status_t statusCode, const char* reason, void *arg);
    static int bufferSizeCallback(jack_nframes_t bufferSize, void *arg);

public:
    // methods implemented from JackInterface:
    virtual jack_client_t * client_by_name(const char *client_name);
    virtual std::list<jack_client_t*> get_clients();
    virtual const char * get_name() const;

    virtual void get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr);
    virtual const char * get_version_string();
    virtual jack_client_t * client_open (const char *client_name, jack_options_t options, jack_status_t *, ...);
    virtual int client_close (jack_client_t *client);
    virtual int client_name_size ();
    virtual char * get_client_name (jack_client_t *client);
    virtual int activate (jack_client_t *client);
    virtual int deactivate (jack_client_t *client);
    virtual int get_client_pid (const char *);
    virtual pthread_t client_thread_id (jack_client_t *client);
    virtual int is_realtime (jack_client_t *client);
    virtual int set_thread_init_callback (jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg);
    virtual void on_shutdown (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg);
    virtual void on_info_shutdown (jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg);
    virtual int set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg);
    virtual int set_freewheel_callback (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg);
    virtual int set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg);
    virtual int set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg);
    virtual int set_client_registration_callback (jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg);
    virtual int set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg);
    virtual int set_port_connect_callback (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg);
    virtual int set_port_rename_callback (jack_client_t *client, JackPortRenameCallback rename_callback, void *arg);
    virtual int set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg);
    virtual int set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg);
    virtual int set_freewheel(jack_client_t *client, int onoff);
    virtual int set_buffer_size (jack_client_t *client, jack_nframes_t nframes);
    virtual jack_nframes_t get_sample_rate (jack_client_t *client);
    virtual jack_nframes_t get_buffer_size (jack_client_t *client);
    virtual float cpu_load (jack_client_t *client);
    virtual jack_port_t * port_register (jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size);
    virtual int port_unregister (jack_client_t *client, jack_port_t *port);
    virtual void * port_get_buffer (jack_port_t *port, jack_nframes_t nframes);
    virtual const char * port_name (const jack_port_t *port);
    virtual const char * port_short_name (const jack_port_t *port);
    virtual int port_flags (const jack_port_t *port);
    virtual const char * port_type (const jack_port_t *port);
    virtual int port_is_mine (const jack_client_t *client, const jack_port_t *port);
    virtual int port_connected (const jack_port_t *port);
    virtual int port_connected_to (const jack_port_t *port, const char *port_name);
    virtual const char ** port_get_connections (const jack_port_t *port);
    virtual const char ** port_get_all_connections (const jack_client_t *client, const jack_port_t *port);
    virtual jack_nframes_t port_get_latency (jack_port_t *port);
    virtual jack_nframes_t port_get_total_latency (jack_client_t *client, jack_port_t *port);
    virtual void port_set_latency (jack_port_t *port, jack_nframes_t nframes);
    virtual int recompute_total_latency (jack_client_t *client, jack_port_t *port);
    virtual int recompute_total_latencies (jack_client_t *client);
    virtual int port_set_name (jack_port_t *port, const char *port_name);
    virtual int port_set_alias (jack_port_t *port, const char *alias);
    virtual int port_unset_alias (jack_port_t *port, const char *alias);
    virtual int port_get_aliases (const jack_port_t *port, char* const aliases[]);
    virtual int port_request_monitor (jack_port_t *port, int onoff);
    virtual int port_request_monitor_by_name (jack_client_t *client, const char *, int onoff);
    virtual int port_ensure_monitor (jack_port_t *port, int onoff);
    virtual int port_monitoring_input (jack_port_t *port);
    virtual int connect (jack_client_t *client, const char *source_port, const char *destination_port);
    virtual int disconnect (jack_client_t *client, const char *source_port, const char *destination_port);
    virtual int port_disconnect (jack_client_t *, jack_port_t *);
    virtual int port_name_size(void);
    virtual int port_type_size(void);
    virtual const char ** get_ports (jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags);
    virtual jack_port_t * port_by_name (jack_client_t *client, const char *port_name);
    virtual jack_port_t * port_by_id (jack_client_t *client, jack_port_id_t port_id);
    virtual jack_nframes_t frames_since_cycle_start (const jack_client_t *client);
    virtual jack_nframes_t frame_time (const jack_client_t *client);
    virtual jack_nframes_t last_frame_time (const jack_client_t *client);
    virtual jack_time_t frames_to_time(const jack_client_t *client, jack_nframes_t nframes);
    virtual jack_nframes_t time_to_frames(const jack_client_t *client, jack_time_t time);
    virtual jack_time_t get_time();
    virtual void set_error_function (void (*func)(const char *));
    virtual void set_info_function (void (*func)(const char *));
    virtual void free(void* ptr);
};

#endif // METAJACKCONTEXTNEW_H
