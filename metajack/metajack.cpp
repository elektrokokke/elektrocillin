#include <jack/jack.h>
#include <stdarg.h>
#include <cassert>
#include "metajackcontext.h"

#ifdef __cplusplus
extern "C"
{
#endif

JackInterface * getJackInterface()
{
    return MetaJackContext::getInstance();
}

JackInterface * getJackInterfaceByClient(const jack_client_t *client) {
    return MetaJackContext::getInstance();
}

JackInterface * getJackInterfaceByPort(const jack_port_t *port) {
    return MetaJackContext::getInstance();
}

void meta_jack_get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr)
{
    getJackInterface()->get_version(major_ptr, minor_ptr, micro_ptr, proto_ptr);
}

const char * meta_jack_get_version_string()
{
    return getJackInterface()->get_version_string();
}

jack_client_t * meta_jack_client_open (const char *client_name, jack_options_t options, jack_status_t *, ...)
{
    return getJackInterface()->client_open(client_name, options, 0);
}

int meta_jack_client_close (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->client_close(client);
}

int meta_jack_client_name_size ()
{
    return getJackInterface()->client_name_size();
}

char * meta_jack_get_client_name (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->get_client_name(client);
}

int meta_jack_activate (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->activate(client);
}

int meta_jack_deactivate (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->deactivate(client);
}

int meta_jack_get_client_pid (const char *name)
{
    return getJackInterface()->get_client_pid(name);
}

pthread_t meta_jack_client_thread_id (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->client_thread_id(client);
}

int meta_jack_is_realtime (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->is_realtime(client);
}

int meta_jack_set_thread_init_callback (jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_thread_init_callback(client, thread_init_callback, arg);
}

void meta_jack_on_shutdown (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg)
{
    getJackInterfaceByClient(client)->on_shutdown(client, shutdown_callback, arg);
}

void meta_jack_on_info_shutdown (jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    getJackInterfaceByClient(client)->on_info_shutdown(client, shutdown_callback, arg);
}

int meta_jack_set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_process_callback(client, process_callback, arg);
}

int meta_jack_set_freewheel_callback (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_freewheel_callback(client, freewheel_callback, arg);
}

int meta_jack_set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_buffer_size_callback(client, bufsize_callback, arg);
}

int meta_jack_set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_sample_rate_callback(client, srate_callback, arg);
}

int meta_jack_set_client_registration_callback (jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_client_registration_callback(client, registration_callback, arg);
}

int meta_jack_set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_port_registration_callback(client, registration_callback, arg);
}

int meta_jack_set_port_connect_callback (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_port_connect_callback(client, connect_callback, arg);
}

int meta_jack_set_port_rename_callback (jack_client_t *client, JackPortRenameCallback rename_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_port_rename_callback(client, rename_callback, arg);
}

int meta_jack_set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_graph_order_callback(client, graph_callback, arg);
}

int meta_jack_set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg)
{
    return getJackInterfaceByClient(client)->set_xrun_callback(client, xrun_callback, arg);
}

int meta_jack_set_freewheel(jack_client_t *client, int onoff)
{
    return getJackInterfaceByClient(client)->set_freewheel(client, onoff);
}

int meta_jack_set_buffer_size (jack_client_t *client, jack_nframes_t nframes)
{
    return getJackInterfaceByClient(client)->set_buffer_size(client, nframes);
}

jack_nframes_t meta_jack_get_sample_rate (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->get_sample_rate(client);
}

jack_nframes_t meta_jack_get_buffer_size (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->get_buffer_size(client);
}

float meta_jack_cpu_load (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->cpu_load(client);
}

jack_port_t * meta_jack_port_register (jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size)
{
    return getJackInterfaceByClient(client)->port_register(client, port_name, port_type, flags, buffer_size);
}

int meta_jack_port_unregister (jack_client_t *client, jack_port_t *port)
{
    return getJackInterfaceByClient(client)->port_unregister(client, port);
}

void * meta_jack_port_get_buffer (jack_port_t *port, jack_nframes_t nframes)
{
    return getJackInterfaceByPort(port)->port_get_buffer(port, nframes);
}

const char * meta_jack_port_name (const jack_port_t *port)
{
    return getJackInterfaceByPort(port)->port_name(port);
}

const char * meta_jack_port_short_name (const jack_port_t *port)
{
    return getJackInterfaceByPort(port)->port_short_name(port);
}

int meta_jack_port_flags (const jack_port_t *port)
{
    return getJackInterfaceByPort(port)->port_flags(port);
}

const char * meta_jack_port_type (const jack_port_t *port)
{
    return getJackInterfaceByPort(port)->port_type(port);
}

int meta_jack_port_is_mine (const jack_client_t *client, const jack_port_t *port)
{
    return getJackInterfaceByClient(client)->port_is_mine(client, port);
}

int meta_jack_port_connected (const jack_port_t *port)
{
    return getJackInterfaceByPort(port)->port_connected(port);
}

int meta_jack_port_connected_to (const jack_port_t *port, const char *port_name)
{
    return getJackInterfaceByPort(port)->port_connected_to(port, port_name);
}

const char ** meta_jack_port_get_connections (const jack_port_t *port)
{
    return getJackInterfaceByPort(port)->port_get_connections(port);
}

const char ** meta_jack_port_get_all_connections (const jack_client_t *client, const jack_port_t *port)
{
    return getJackInterfaceByClient(client)->port_get_all_connections(client, port);
}

jack_nframes_t meta_jack_port_get_latency (jack_port_t *port)
{
   return getJackInterfaceByPort(port)->port_get_latency(port);
}

jack_nframes_t meta_jack_port_get_total_latency (jack_client_t *client, jack_port_t *port)
{
    return getJackInterfaceByClient(client)->port_get_total_latency(client, port);
}

void meta_jack_port_set_latency (jack_port_t *port, jack_nframes_t nframes)
{
    return getJackInterfaceByPort(port)->port_set_latency(port, nframes);
}

int meta_jack_recompute_total_latency (jack_client_t *client, jack_port_t *port)
{
    return getJackInterfaceByClient(client)->recompute_total_latency(client, port);
}

int meta_jack_recompute_total_latencies (jack_client_t *client)
{
    return getJackInterfaceByClient(client)->recompute_total_latencies(client);
}

int meta_jack_port_set_name (jack_port_t *port, const char *port_name)
{
    return getJackInterfaceByPort(port)->port_set_name(port, port_name);
}

int meta_jack_port_set_alias (jack_port_t *port, const char *alias)
{
    return getJackInterfaceByPort(port)->port_set_alias(port, alias);
}

int meta_jack_port_unset_alias (jack_port_t *port, const char *alias)
{
    return getJackInterfaceByPort(port)->port_unset_alias(port, alias);
}

int meta_jack_port_get_aliases (const jack_port_t *port, char* const aliases[])
{
    return getJackInterfaceByPort(port)->port_get_aliases(port, aliases);
}

int meta_jack_port_request_monitor (jack_port_t *port, int onoff)
{
    return getJackInterfaceByPort(port)->port_request_monitor(port, onoff);
}

int meta_jack_port_request_monitor_by_name (jack_client_t *client, const char *port_name, int onoff)
{
    return getJackInterfaceByClient(client)->port_request_monitor_by_name(client, port_name, onoff);
}

int meta_jack_port_ensure_monitor (jack_port_t *port, int onoff)
{
    return getJackInterfaceByPort(port)->port_ensure_monitor(port, onoff);
}

int meta_jack_port_monitoring_input (jack_port_t *port)
{
    return getJackInterfaceByPort(port)->port_monitoring_input(port);
}

int meta_jack_connect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return getJackInterfaceByClient(client)->connect(client, source_port, destination_port);
}

int meta_jack_disconnect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return getJackInterfaceByClient(client)->disconnect(client, source_port, destination_port);
}

int meta_jack_port_disconnect (jack_client_t *client, jack_port_t *port)
{
    return getJackInterfaceByClient(client)->port_disconnect(client, port);
}

int meta_jack_port_name_size()
{
    return getJackInterface()->port_name_size();
}

int meta_jack_port_type_size()
{
    return getJackInterface()->port_type_size();
}

const char ** meta_jack_get_ports (jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags)
{
    return getJackInterfaceByClient(client)->get_ports(client, port_name_pattern, type_name_pattern, flags);
}

jack_port_t * meta_jack_port_by_name (jack_client_t *client, const char *port_name)
{
    return getJackInterfaceByClient(client)->port_by_name(client, port_name);
}

jack_port_t * meta_jack_port_by_id (jack_client_t *client, jack_port_id_t port_id)
{
    return getJackInterfaceByClient(client)->port_by_id(client, port_id);
}

jack_nframes_t meta_jack_frames_since_cycle_start (const jack_client_t *client)
{
    return getJackInterfaceByClient(client)->frames_since_cycle_start(client);
}

jack_nframes_t meta_jack_frame_time (const jack_client_t *client)
{
    return getJackInterfaceByClient(client)->frame_time(client);
}

jack_nframes_t meta_jack_last_frame_time (const jack_client_t *client)
{
    return getJackInterfaceByClient(client)->last_frame_time(client);
}

jack_time_t meta_jack_frames_to_time(const jack_client_t *client, jack_nframes_t nframes)
{
    return getJackInterfaceByClient(client)->frames_to_time(client, nframes);
}

jack_nframes_t meta_jack_time_to_frames(const jack_client_t *client, jack_time_t time)
{
    return getJackInterfaceByClient(client)->time_to_frames(client, time);
}

jack_time_t meta_jack_get_time()
{
    return getJackInterface()->get_time();
}

void meta_jack_set_error_function (void (*func)(const char *))
{
    getJackInterface()->set_error_function(func);
}

void meta_jack_set_info_function (void (*func)(const char *))
{
    getJackInterface()->set_info_function(func);
}

void meta_jack_free(void* ptr)
{
    getJackInterface()->free(ptr);
}

#ifdef __cplusplus
}
#endif
