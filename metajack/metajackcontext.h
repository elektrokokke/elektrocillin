#ifndef METAJACKCONTEXTNEW_H
#define METAJACKCONTEXTNEW_H

#include "metajackclient.h"
#include "metajackport.h"
#include "callbackhandlers.h"
#include "jackringbuffer.h"
#include <jack/midiport.h>
#include <map>
#include <QWaitCondition>
#include <QMutex>

class MetaJackContext
{
public:
    struct MetaJackContextMidiBufferHead {
        size_t bufferSize, midiEventCount, midiDataSize;
        jack_nframes_t lostMidiEvents;
    };

    static MetaJackContext *instance;
    static MetaJackContext instance_;

    MetaJackContext(const std::string &name);
    ~MetaJackContext();

    bool isActive() const;

    static size_t getClientNameSize();
    static size_t getPortNameSize();
    static size_t getPortTypeSize();

    jack_port_id_t createUniquePortId();

    // create and delete clients:
    MetaJackClient * openClient(const std::string &name, jack_options_t options);
    bool closeClient(MetaJackClient *client);
    // methods to change the process graph
    // each of the following methods has a corresponding method which is called from the process thread
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

    int get_pid();
    pthread_t get_thread_id();
    bool is_realtime();

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
    class MetaJackGraphEvent {
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
    jack_nframes_t bufferSize;
    jack_port_id_t uniquePortId;
    std::map<std::string, MetaJackClient*> clients;
    std::map<std::string, MetaJackPort*> portsByName;
    std::map<jack_port_id_t, MetaJackPort*> portsById;
    std::set<MetaJackClientProcess*> activeClients;
    JackRingBuffer<MetaJackGraphEvent> graphChangesRingBuffer;
    QWaitCondition waitCondition;
    QMutex waitMutex;
    bool shutdown;

    void setProcessCallback(MetaJackClientProcess *client, JackProcessCallback processCallback, void *processCallbackArgument);
    void activateClient(MetaJackClientProcess *client);
    void deactivateClient(MetaJackClientProcess *client);
    void registerPort(MetaJackClientProcess *client, MetaJackPortProcess *port);
    void unregisterPort(MetaJackPortProcess *port);
    void renamePort(MetaJackPortProcess *port, const std::string &shortName);
    void connectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest);
    void disconnectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest);

    // signal graph change:
    void sendGraphChangeEvent(const MetaJackGraphEvent &event);

    int process(jack_nframes_t nframes);
    static int process(jack_nframes_t nframes, void *arg);
    static void infoShutdownCallback(jack_status_t statusCode, const char* reason, void *arg);
    static int bufferSizeCallback(jack_nframes_t bufferSize, void *arg);
};

#endif // METAJACKCONTEXTNEW_H
