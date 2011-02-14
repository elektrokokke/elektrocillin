#ifndef METAJACKCONTEXTNEW_H
#define METAJACKCONTEXTNEW_H

#include "metajackclient.h"
#include "metajackport.h"
#include "jackringbuffer.h"
#include <jack/midiport.h>
#include <map>
#include <QWaitCondition>
#include <QMutex>

class MetaJackContextNew
{
public:
    struct MetaJackContextMidiBufferHead {
        size_t bufferSize, midiEventCount, midiDataSize;
        jack_nframes_t lostMidiEvents;
    };


    static MetaJackContextNew *instance;
    static MetaJackContextNew instance_;

    MetaJackContextNew(const std::string &name);
    ~MetaJackContextNew();

    bool isActive() const;

    static size_t getClientNameSize();
    static size_t getPortNameSize();
    static size_t getPortTypeSize();

    jack_port_id_t createUniquePortId();

    // create and delete clients:
    MetaJackClientNew * openClient(const std::string &name, jack_options_t options);
    bool closeClient(MetaJackClientNew *client);
    // methods to change the process graph
    // each of the following methods has a corresponding method which is called from the process thread
    bool setProcessCallback(MetaJackClientNew *client, JackProcessCallback processCallback, void *processCallbackArgument);
    bool activateClient(MetaJackClientNew *client);
    bool deactivateClient(MetaJackClientNew *client);
    MetaJackPortNew * registerPort(MetaJackClientNew *client, const std::string & shortName, const std::string &type, unsigned long flags, unsigned long buffer_size);
    bool unregisterPort(MetaJackPortNew *port);
    bool renamePort(MetaJackPortNew *port, const std::string &shortName);
    bool connectPorts(const std::string &source_port, const std::string &destination_port);
    bool disconnectPorts(const std::string &source_port, const std::string &destination_port);

    // client- and port-related methods:
    const char ** getPortsByPattern(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags);
    MetaJackPortNew * getPortByName(const std::string &name) const;
    MetaJackPortNew * getPortById(jack_port_id_t id);

    int get_pid();
    pthread_t get_thread_id();
    bool is_realtime();

    // callback setting methods:
    int     set_thread_init_callback (MetaJackClientNew *client, JackThreadInitCallback thread_init_callback, void *arg);
    void    set_shutdown_callback (MetaJackClientNew *client, JackShutdownCallback shutdown_callback, void *arg);
    void    set_info_shutdown_callback (MetaJackClientNew *client, JackInfoShutdownCallback shutdown_callback, void *arg);
    int     set_freewheel_callback (MetaJackClientNew *client, JackFreewheelCallback freewheel_callback, void *arg);
    int     set_buffer_size_callback (MetaJackClientNew *client, JackBufferSizeCallback bufsize_callback, void *arg);
    int     set_sample_rate_callback (MetaJackClientNew *client, JackSampleRateCallback srate_callback, void *arg);
    int     set_client_registration_callback (MetaJackClientNew *, JackClientRegistrationCallback registration_callback, void *arg);
    int     set_port_registration_callback (MetaJackClientNew *, JackPortRegistrationCallback registration_callback, void *arg);
    int     set_port_connect_callback (MetaJackClientNew *, JackPortConnectCallback connect_callback, void *arg);
    int     set_port_rename_callback (MetaJackClientNew *, JackPortRenameCallback rename_callback, void *arg);
    int     set_graph_order_callback (MetaJackClientNew *, JackGraphOrderCallback graph_callback, void *);
    int     set_xrun_callback (MetaJackClientNew *, JackXRunCallback xrun_callback, void *arg);

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
    static bool                compare_midi_events(const jack_midi_event_t &event1, const jack_midi_event_t &event2);


    // for all methods returning const char **:
    void free(void* ptr);

private:
    class MetaJackGraphEventNew {
    public:
        enum {
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
        JackProcessCallback processCallback;
        void * processCallbackArgument;
        std::string shortName;
    };

    jack_client_t *wrapperClient;
    jack_port_id_t uniquePortId;
    std::map<std::string, MetaJackClientNew*> clients;
    std::map<std::string, MetaJackPortNew*> portsByName;
    std::map<jack_port_id_t, MetaJackPortNew*> portsById;
    std::map<MetaJackClientNew*, std::pair<JackThreadInitCallback, void*> > threadInitCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackShutdownCallback, void*> > shutdownCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackInfoShutdownCallback, void*> > infoShutdownCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackFreewheelCallback, void*> > freewheelCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackBufferSizeCallback, void*> > bufferSizeCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackSampleRateCallback, void*> > sampleRateCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackClientRegistrationCallback, void*> > clientRegistrationCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackPortRegistrationCallback, void*> > portRegistrationCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackPortConnectCallback, void*> > portConnectCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackPortRenameCallback, void*> > portRenameCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackGraphOrderCallback, void*> > graphOrderCallbacks;
    std::map<MetaJackClientNew*, std::pair<JackXRunCallback, void*> > xRunCallbacks;
    std::set<MetaJackClientProcess*> activeClients;
    JackRingBuffer<MetaJackGraphEventNew> graphChangesRingBuffer;
    QWaitCondition waitCondition;
    QMutex waitMutex;

    void setProcessCallback(MetaJackClientProcess *client, JackProcessCallback processCallback, void *processCallbackArgument);
    void activateClient(MetaJackClientProcess *client);
    void deactivateClient(MetaJackClientProcess *client);
    void registerPort(MetaJackClientProcess *client, MetaJackPortProcess *port);
    void unregisterPort(MetaJackPortProcess *port);
    void renamePort(MetaJackPortProcess *port, const std::string &shortName);
    void connectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest);
    void disconnectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest);

    // invoke callbacks:
    void invokeJackClientRegistrationCallbacks(const std::string &clientName, bool registered);
    void invokeJackPortRegistrationCallbacks(jack_port_id_t id, bool registered);
    void invokeJackPortRenameCallbacks(jack_port_id_t id, const std::string &oldName, const std::string &newName);
    void invokeJackPortConnectCallbacks(jack_port_id_t a, jack_port_id_t b, bool connected);
    // signal graph change:
    void sendGraphChangeEvent(const MetaJackGraphEventNew &event);

    int process(jack_nframes_t nframes);
    static int process(jack_nframes_t nframes, void *arg);
};

#endif // METAJACKCONTEXTNEW_H
