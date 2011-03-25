#include "realjackcontext.h"
#include <jack/jack.h>
#include <jack/transport.h>

RealJackContext::RealJackContext() :
    name("Jack server")
{
}

jack_client_t * RealJackContext::client_by_name(const char *client_name)
{
    std::map<std::string, jack_client_t*>::iterator find = clients.find(client_name);
    if (find != clients.end()) {
        return find->second;
    } else {
        return 0;
    }
}

std::list<jack_client_t*> RealJackContext::get_clients()
{
    std::list<jack_client_t*> clientList;
    for (std::map<std::string, jack_client_t*>::iterator i = clients.begin(); i != clients.end(); i++) {
        clientList.push_back(i->second);
    }
    return clientList;
}

const char * RealJackContext::get_name() const
{
    return name.c_str();
}

void RealJackContext::get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr)
{
    jack_get_version(major_ptr, minor_ptr, micro_ptr, proto_ptr);
}

const char * RealJackContext::get_version_string()
{
    return jack_get_version_string();
}

jack_client_t * RealJackContext::client_open (const char *client_name, jack_options_t options, jack_status_t *, ...)
{
    // TODO: figure out how the ellipsis arguments can be implemented... probably have to consider each possible case
    jack_client_t *client = jack_client_open(client_name, options, 0);
    if (client) {
        clients[jack_get_client_name(client)] = client;
    }
    return client;
}

int RealJackContext::client_close (jack_client_t *client)
{
    std::string client_name = jack_get_client_name(client);
    int returnValue = jack_client_close(client);
    if (returnValue == 0) {
        clients.erase(client_name);
    }
    return returnValue;
}

int RealJackContext::client_name_size ()
{
    return jack_client_name_size();
}

char * RealJackContext::get_client_name (jack_client_t *client)
{
    return jack_get_client_name(client);
}

int RealJackContext::activate (jack_client_t *client)
{
    return jack_activate(client);
}

int RealJackContext::deactivate (jack_client_t *client)
{
    return jack_deactivate(client);
}

int RealJackContext::get_client_pid (const char *name)
{
    return jack_get_client_pid(name);
}

pthread_t RealJackContext::client_thread_id (jack_client_t *client)
{
    return jack_client_thread_id(client);
}

int RealJackContext::is_realtime (jack_client_t *client)
{
    return jack_is_realtime(client);
}

int RealJackContext::set_thread_init_callback (jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    return jack_set_thread_init_callback(client, thread_init_callback, arg);
}

void RealJackContext::on_shutdown (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg)
{
    jack_on_shutdown(client, shutdown_callback, arg);
}

void RealJackContext::on_info_shutdown (jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    jack_on_info_shutdown(client, shutdown_callback, arg);
}

int RealJackContext::set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg)
{
    return jack_set_process_callback(client, process_callback, arg);
}

int RealJackContext::set_freewheel_callback (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    return jack_set_freewheel_callback(client, freewheel_callback, arg);
}

int RealJackContext::set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    return jack_set_buffer_size_callback(client, bufsize_callback, arg);
}

int RealJackContext::set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg)
{
    return jack_set_sample_rate_callback(client, srate_callback, arg);
}

int RealJackContext::set_client_registration_callback (jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    return jack_set_client_registration_callback(client, registration_callback, arg);
}

int RealJackContext::set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    return jack_set_port_registration_callback(client, registration_callback, arg);
}

int RealJackContext::set_port_connect_callback (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg)
{
    return jack_set_port_connect_callback(client, connect_callback, arg);
}

int RealJackContext::set_port_rename_callback (jack_client_t *client, JackPortRenameCallback rename_callback, void *arg)
{
    return jack_set_port_rename_callback(client, rename_callback, arg);
}

int RealJackContext::set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg)
{
    return jack_set_graph_order_callback(client, graph_callback, arg);
}

int RealJackContext::set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg)
{
    return jack_set_xrun_callback(client, xrun_callback, arg);
}

int RealJackContext::set_freewheel(jack_client_t *client, int onoff)
{
    return jack_set_freewheel(client, onoff);
}

int RealJackContext::set_buffer_size (jack_client_t *client, jack_nframes_t nframes)
{
    return jack_set_buffer_size(client, nframes);
}

jack_nframes_t RealJackContext::get_sample_rate (jack_client_t *client)
{
    return jack_get_sample_rate(client);
}

jack_nframes_t RealJackContext::get_buffer_size (jack_client_t *client)
{
    return jack_get_buffer_size(client);
}

float RealJackContext::cpu_load (jack_client_t *client)
{
    return jack_cpu_load(client);
}

jack_port_t * RealJackContext::port_register (jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size)
{
    return jack_port_register(client, port_name, port_type, flags, buffer_size);
}

int RealJackContext::port_unregister (jack_client_t *client, jack_port_t *port)
{
    return jack_port_unregister(client, port);
}

void * RealJackContext::port_get_buffer (jack_port_t *port, jack_nframes_t nframes)
{
    return jack_port_get_buffer(port, nframes);
}

const char * RealJackContext::port_name (const jack_port_t *port)
{
    return jack_port_name(port);
}

const char * RealJackContext::port_short_name (const jack_port_t *port)
{
    return jack_port_short_name(port);
}

int RealJackContext::port_flags (const jack_port_t *port)
{
    return jack_port_flags(port);
}

const char * RealJackContext::port_type (const jack_port_t *port)
{
    return jack_port_type(port);
}

int RealJackContext::port_is_mine (const jack_client_t *client, const jack_port_t *port)
{
    return jack_port_is_mine(client, port);
}

int RealJackContext::port_connected (const jack_port_t *port)
{
    return jack_port_connected(port);
}

int RealJackContext::port_connected_to (const jack_port_t *port, const char *port_name)
{
    return jack_port_connected_to(port, port_name);
}

const char ** RealJackContext::port_get_connections (const jack_port_t *port)
{
    return jack_port_get_connections(port);
}

const char ** RealJackContext::port_get_all_connections (const jack_client_t *client, const jack_port_t *port)
{
    return jack_port_get_all_connections(client, port);
}

jack_nframes_t RealJackContext::port_get_latency (jack_port_t *port)
{
    return jack_port_get_latency(port);
}

jack_nframes_t RealJackContext::port_get_total_latency (jack_client_t *client, jack_port_t *port)
{
    return jack_port_get_total_latency(client, port);
}

void RealJackContext::port_set_latency (jack_port_t *port, jack_nframes_t nframes) {
    jack_port_set_latency(port, nframes);
}

int RealJackContext::recompute_total_latency (jack_client_t *client, jack_port_t *port)
{
    return jack_recompute_total_latency(client, port);
}

int RealJackContext::recompute_total_latencies (jack_client_t *client)
{
    return jack_recompute_total_latencies(client);
}

int RealJackContext::port_set_name (jack_port_t *port, const char *port_name)
{
    return jack_port_set_name(port, port_name);
}

int RealJackContext::port_set_alias (jack_port_t *port, const char *alias) {
    return jack_port_set_alias(port, alias);
}

int RealJackContext::port_unset_alias (jack_port_t *port, const char *alias)
{
    return jack_port_unset_alias(port, alias);
}

int RealJackContext::port_get_aliases (const jack_port_t *port, char* const aliases[])
{
    return jack_port_get_aliases(port, aliases);
}

int RealJackContext::port_request_monitor (jack_port_t *port, int onoff)
{
    return jack_port_request_monitor(port, onoff);
}

int RealJackContext::port_request_monitor_by_name (jack_client_t *client, const char *port_name, int onoff)
{
    return jack_port_request_monitor_by_name(client, port_name, onoff);
}

int RealJackContext::port_ensure_monitor (jack_port_t *port, int onoff)
{
    return jack_port_ensure_monitor(port, onoff);
}

int RealJackContext::port_monitoring_input (jack_port_t *port)
{
    return jack_port_monitoring_input(port);
}

int RealJackContext::connect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return jack_connect(client, source_port, destination_port);
}

int RealJackContext::disconnect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return jack_disconnect(client, source_port, destination_port);
}

int RealJackContext::port_disconnect (jack_client_t *client, jack_port_t *port)
{
    return jack_port_disconnect(client, port);
}

int RealJackContext::port_name_size()
{
    return jack_port_name_size();
}

int RealJackContext::port_type_size()
{
    return jack_port_type_size();
}

const char ** RealJackContext::get_ports (jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags)
{
    return jack_get_ports(client, port_name_pattern, type_name_pattern, flags);
}

jack_port_t * RealJackContext::port_by_name (jack_client_t *client, const char *port_name)
{
    return jack_port_by_name(client, port_name);
}

jack_port_t * RealJackContext::port_by_id (jack_client_t *client, jack_port_id_t port_id)
{
    return jack_port_by_id(client, port_id);
}

jack_nframes_t RealJackContext::frames_since_cycle_start (const jack_client_t *client)
{
    return jack_frames_since_cycle_start(client);
}

jack_nframes_t RealJackContext::frame_time (const jack_client_t *client)
{
    return jack_frame_time(client);
}

jack_nframes_t RealJackContext::last_frame_time (const jack_client_t *client)
{
    return jack_last_frame_time(client);
}

jack_time_t RealJackContext::frames_to_time(const jack_client_t *client, jack_nframes_t nframes)
{
    return jack_frames_to_time(client, nframes);
}

jack_nframes_t RealJackContext::time_to_frames(const jack_client_t *client, jack_time_t time)
{
    return jack_time_to_frames(client, time);
}

jack_time_t RealJackContext::get_time()
{
    return jack_get_time();
}

void RealJackContext::set_error_function (void (*func)(const char *))
{
    jack_set_error_function(func);
}

void RealJackContext::set_info_function (void (*func)(const char *))
{
    jack_set_info_function(func);
}

void RealJackContext::free(void* ptr)
{
    jack_free(ptr);
}

int RealJackContext::release_timebase (jack_client_t *client)
{
    return jack_release_timebase(client);
}

int RealJackContext::set_sync_callback (jack_client_t *client, JackSyncCallback sync_callback, void *arg)
{
    return jack_set_sync_callback(client, sync_callback, arg);
}

int RealJackContext::set_sync_timeout (jack_client_t *client, jack_time_t timeout)
{
    return jack_set_sync_timeout(client, timeout);
}

int RealJackContext::set_timebase_callback (jack_client_t *client, int conditional, JackTimebaseCallback timebase_callback, void *arg)
{
    return jack_set_timebase_callback(client, conditional, timebase_callback, arg);
}

int RealJackContext::transport_locate (jack_client_t *client, jack_nframes_t frame)
{
    return jack_transport_locate(client, frame);
}

jack_transport_state_t RealJackContext::transport_query (const jack_client_t *client, jack_position_t *pos)
{
    return jack_transport_query(client, pos);
}

jack_nframes_t RealJackContext::get_current_transport_frame (const jack_client_t *client)
{
    return jack_get_current_transport_frame(client);
}

int RealJackContext::transport_reposition (jack_client_t *client, jack_position_t *pos)
{
    return jack_transport_reposition(client, pos);
}

void RealJackContext::transport_start (jack_client_t *client)
{
    jack_transport_start(client);
}

void RealJackContext::transport_stop (jack_client_t *client)
{
    jack_transport_stop(client);
}

void RealJackContext::get_transport_info (jack_client_t *client, jack_transport_info_t *tinfo)
{
    jack_get_transport_info(client, tinfo);
}

void RealJackContext::set_transport_info (jack_client_t *client, jack_transport_info_t *tinfo)
{
    jack_set_transport_info(client, tinfo);
}

jack_nframes_t RealJackContext::midi_get_event_count(void* port_buffer)
{
    return jack_midi_get_event_count(port_buffer);
}

int RealJackContext::midi_event_get(jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index)
{
    return jack_midi_event_get(event, port_buffer, event_index);
}

void RealJackContext::midi_clear_buffer(void *port_buffer)
{
    jack_midi_clear_buffer(port_buffer);
}

size_t RealJackContext::midi_max_event_size(void* port_buffer)
{
    return jack_midi_max_event_size(port_buffer);
}

jack_midi_data_t * RealJackContext::midi_event_reserve(void *port_buffer, jack_nframes_t time, size_t data_size)
{
    return jack_midi_event_reserve(port_buffer, time, data_size);
}

int RealJackContext::midi_event_write(void *port_buffer, jack_nframes_t time, const jack_midi_data_t *data, size_t data_size)
{
    return jack_midi_event_write(port_buffer, time, data, data_size);
}

jack_nframes_t RealJackContext::midi_get_lost_event_count(void *port_buffer)
{
    return jack_midi_get_lost_event_count(port_buffer);
}
