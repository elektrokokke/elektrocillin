#ifndef METAJACKINTERFACE_H
#define METAJACKINTERFACE_H

#include <jack/types.h>

class JackContext {
public:
    // auxiliary methods:
    virtual jack_client_t * client_by_name(const char *client_name) = 0;

    // Jack API methods:
    virtual void get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr) = 0;
    virtual const char * get_version_string() = 0;
    virtual jack_client_t * client_open (const char *client_name, jack_options_t options, jack_status_t *, ...) = 0;
    virtual int client_close (jack_client_t *client) = 0;
    virtual int client_name_size () = 0;
    virtual char * get_client_name (jack_client_t *client) = 0;
    virtual int activate (jack_client_t *client) = 0;
    virtual int deactivate (jack_client_t *client) = 0;
    virtual int get_client_pid (const char *) = 0;
    virtual pthread_t client_thread_id (jack_client_t *client) = 0;
    virtual int is_realtime (jack_client_t *client) = 0;
    virtual int set_thread_init_callback (jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg) = 0;
    virtual void on_shutdown (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg) = 0;
    virtual void on_info_shutdown (jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg) = 0;
    virtual int set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg) = 0;
    virtual int set_freewheel_callback (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg) = 0;
    virtual int set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg) = 0;
    virtual int set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg) = 0;
    virtual int set_client_registration_callback (jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg) = 0;
    virtual int set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg) = 0;
    virtual int set_port_connect_callback (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg) = 0;
    virtual int set_port_rename_callback (jack_client_t *client, JackPortRenameCallback rename_callback, void *arg) = 0;
    virtual int set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg) = 0;
    virtual int set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg) = 0;
    virtual int set_freewheel(jack_client_t *client, int onoff) = 0;
    virtual int set_buffer_size (jack_client_t *client, jack_nframes_t nframes) = 0;
    virtual jack_nframes_t get_sample_rate (jack_client_t *client) = 0;
    virtual jack_nframes_t get_buffer_size (jack_client_t *client) = 0;
    virtual float cpu_load (jack_client_t *client) = 0;
    virtual jack_port_t * port_register (jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size) = 0;
    virtual int port_unregister (jack_client_t *client, jack_port_t *port) = 0;
    virtual void * port_get_buffer (jack_port_t *port, jack_nframes_t nframes) = 0;
    virtual const char * port_name (const jack_port_t *port) = 0;
    virtual const char * port_short_name (const jack_port_t *port) = 0;
    virtual int port_flags (const jack_port_t *port) = 0;
    virtual const char * port_type (const jack_port_t *port) = 0;
    virtual int port_is_mine (const jack_client_t *client, const jack_port_t *port) = 0;
    virtual int port_connected (const jack_port_t *port) = 0;
    virtual int port_connected_to (const jack_port_t *port, const char *port_name) = 0;
    virtual const char ** port_get_connections (const jack_port_t *port) = 0;
    virtual const char ** port_get_all_connections (const jack_client_t *client, const jack_port_t *port) = 0;
    virtual jack_nframes_t port_get_latency (jack_port_t *) = 0;
    virtual jack_nframes_t port_get_total_latency (jack_client_t *, jack_port_t *) = 0;
    virtual void port_set_latency (jack_port_t *, jack_nframes_t) = 0;
    virtual int recompute_total_latency (jack_client_t *, jack_port_t*) = 0;
    virtual int recompute_total_latencies (jack_client_t *) = 0;
    virtual int port_set_name (jack_port_t *port, const char *port_name) = 0;
    virtual int port_set_alias (jack_port_t *port, const char *alias) = 0;
    virtual int port_unset_alias (jack_port_t *port, const char *alias) = 0;
    virtual int port_get_aliases (const jack_port_t *port, char* const aliases[]) = 0;
    virtual int port_request_monitor (jack_port_t *port, int onoff) = 0;
    virtual int port_request_monitor_by_name (jack_client_t *client, const char *port_name, int onoff) = 0;
    virtual int port_ensure_monitor (jack_port_t *port, int onoff) = 0;
    virtual int port_monitoring_input (jack_port_t *port) = 0;
    virtual int connect (jack_client_t *client, const char *source_port, const char *destination_port) = 0;
    virtual int disconnect (jack_client_t *client, const char *source_port, const char *destination_port) = 0;
    virtual int port_disconnect (jack_client_t *, jack_port_t *) = 0;
    virtual int port_name_size() = 0;
    virtual int port_type_size() = 0;
    virtual const char ** get_ports (jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags) = 0;
    virtual jack_port_t * port_by_name (jack_client_t *client, const char *port_name) = 0;
    virtual jack_port_t * port_by_id (jack_client_t *client, jack_port_id_t port_id) = 0;
    virtual jack_nframes_t frames_since_cycle_start (const jack_client_t *client) = 0;
    virtual jack_nframes_t frame_time (const jack_client_t *client) = 0;
    virtual jack_nframes_t last_frame_time (const jack_client_t *client) = 0;
    virtual jack_time_t frames_to_time(const jack_client_t *client, jack_nframes_t nframes) = 0;
    virtual jack_nframes_t time_to_frames(const jack_client_t *client, jack_time_t time) = 0;
    virtual jack_time_t get_time() = 0;
    virtual void set_error_function (void (*func)(const char *)) = 0;
    virtual void set_info_function (void (*func)(const char *)) = 0;
    virtual void free(void* ptr) = 0;
};

#endif // METAJACKINTERFACE_H
