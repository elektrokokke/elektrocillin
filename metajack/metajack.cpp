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

#include <jack/jack.h>
#include <stdarg.h>
#include <cassert>
#include "recursivejackcontext.h"

#ifdef __cplusplus
extern "C"
{
#endif

jack_client_t * meta_jack_client_by_name(const char *client_name)
{
    return RecursiveJackContext::getInstance()->client_by_name(client_name);
}

void meta_jack_get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr)
{
    RecursiveJackContext::getInstance()->get_version(major_ptr, minor_ptr, micro_ptr, proto_ptr);
}

const char * meta_jack_get_version_string()
{
    return RecursiveJackContext::getInstance()->get_version_string();
}

jack_client_t * meta_jack_client_open (const char *client_name, jack_options_t options, jack_status_t *, ...)
{
    return RecursiveJackContext::getInstance()->client_open(client_name, options, 0);
}

int meta_jack_client_close (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->client_close(client);
}

int meta_jack_client_name_size ()
{
    return RecursiveJackContext::getInstance()->client_name_size();
}

char * meta_jack_get_client_name (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->get_client_name(client);
}

int meta_jack_activate (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->activate(client);
}

int meta_jack_deactivate (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->deactivate(client);
}

int meta_jack_get_client_pid (const char *name)
{
    return RecursiveJackContext::getInstance()->get_client_pid(name);
}

pthread_t meta_jack_client_thread_id (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->client_thread_id(client);
}

int meta_jack_is_realtime (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->is_realtime(client);
}

int meta_jack_set_thread_init_callback (jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_thread_init_callback(client, thread_init_callback, arg);
}

void meta_jack_on_shutdown (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg)
{
    RecursiveJackContext::getInstance()->on_shutdown(client, shutdown_callback, arg);
}

void meta_jack_on_info_shutdown (jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    RecursiveJackContext::getInstance()->on_info_shutdown(client, shutdown_callback, arg);
}

int meta_jack_set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_process_callback(client, process_callback, arg);
}

int meta_jack_set_freewheel_callback (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_freewheel_callback(client, freewheel_callback, arg);
}

int meta_jack_set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_buffer_size_callback(client, bufsize_callback, arg);
}

int meta_jack_set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_sample_rate_callback(client, srate_callback, arg);
}

int meta_jack_set_client_registration_callback (jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_client_registration_callback(client, registration_callback, arg);
}

int meta_jack_set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_port_registration_callback(client, registration_callback, arg);
}

int meta_jack_set_port_connect_callback (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_port_connect_callback(client, connect_callback, arg);
}

int meta_jack_set_port_rename_callback (jack_client_t *client, JackPortRenameCallback rename_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_port_rename_callback(client, rename_callback, arg);
}

int meta_jack_set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_graph_order_callback(client, graph_callback, arg);
}

int meta_jack_set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_xrun_callback(client, xrun_callback, arg);
}

int meta_jack_set_freewheel(jack_client_t *client, int onoff)
{
    return RecursiveJackContext::getInstance()->set_freewheel(client, onoff);
}

int meta_jack_set_buffer_size (jack_client_t *client, jack_nframes_t nframes)
{
    return RecursiveJackContext::getInstance()->set_buffer_size(client, nframes);
}

jack_nframes_t meta_jack_get_sample_rate (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->get_sample_rate(client);
}

jack_nframes_t meta_jack_get_buffer_size (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->get_buffer_size(client);
}

float meta_jack_cpu_load (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->cpu_load(client);
}

jack_port_t * meta_jack_port_register (jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size)
{
    return RecursiveJackContext::getInstance()->port_register(client, port_name, port_type, flags, buffer_size);
}

int meta_jack_port_unregister (jack_client_t *client, jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_unregister(client, port);
}

void * meta_jack_port_get_buffer (jack_port_t *port, jack_nframes_t nframes)
{
    return RecursiveJackContext::getInstance()->port_get_buffer(port, nframes);
}

const char * meta_jack_port_name (const jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_name(port);
}

const char * meta_jack_port_short_name (const jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_short_name(port);
}

int meta_jack_port_flags (const jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_flags(port);
}

const char * meta_jack_port_type (const jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_type(port);
}

int meta_jack_port_is_mine (const jack_client_t *client, const jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_is_mine(client, port);
}

int meta_jack_port_connected (const jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_connected(port);
}

int meta_jack_port_connected_to (const jack_port_t *port, const char *port_name)
{
    return RecursiveJackContext::getInstance()->port_connected_to(port, port_name);
}

const char ** meta_jack_port_get_connections (const jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_get_connections(port);
}

const char ** meta_jack_port_get_all_connections (const jack_client_t *client, const jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_get_all_connections(client, port);
}

jack_nframes_t meta_jack_port_get_latency (jack_port_t *port)
{
   return RecursiveJackContext::getInstance()->port_get_latency(port);
}

jack_nframes_t meta_jack_port_get_total_latency (jack_client_t *client, jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_get_total_latency(client, port);
}

void meta_jack_port_set_latency (jack_port_t *port, jack_nframes_t nframes)
{
    return RecursiveJackContext::getInstance()->port_set_latency(port, nframes);
}

int meta_jack_recompute_total_latency (jack_client_t *client, jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->recompute_total_latency(client, port);
}

int meta_jack_recompute_total_latencies (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->recompute_total_latencies(client);
}

int meta_jack_port_set_name (jack_port_t *port, const char *port_name)
{
    return RecursiveJackContext::getInstance()->port_set_name(port, port_name);
}

int meta_jack_port_set_alias (jack_port_t *port, const char *alias)
{
    return RecursiveJackContext::getInstance()->port_set_alias(port, alias);
}

int meta_jack_port_unset_alias (jack_port_t *port, const char *alias)
{
    return RecursiveJackContext::getInstance()->port_unset_alias(port, alias);
}

int meta_jack_port_get_aliases (const jack_port_t *port, char* const aliases[])
{
    return RecursiveJackContext::getInstance()->port_get_aliases(port, aliases);
}

int meta_jack_port_request_monitor (jack_port_t *port, int onoff)
{
    return RecursiveJackContext::getInstance()->port_request_monitor(port, onoff);
}

int meta_jack_port_request_monitor_by_name (jack_client_t *client, const char *port_name, int onoff)
{
    return RecursiveJackContext::getInstance()->port_request_monitor_by_name(client, port_name, onoff);
}

int meta_jack_port_ensure_monitor (jack_port_t *port, int onoff)
{
    return RecursiveJackContext::getInstance()->port_ensure_monitor(port, onoff);
}

int meta_jack_port_monitoring_input (jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_monitoring_input(port);
}

int meta_jack_connect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return RecursiveJackContext::getInstance()->connect(client, source_port, destination_port);
}

int meta_jack_disconnect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return RecursiveJackContext::getInstance()->disconnect(client, source_port, destination_port);
}

int meta_jack_port_disconnect (jack_client_t *client, jack_port_t *port)
{
    return RecursiveJackContext::getInstance()->port_disconnect(client, port);
}

int meta_jack_port_name_size()
{
    return RecursiveJackContext::getInstance()->port_name_size();
}

int meta_jack_port_type_size()
{
    return RecursiveJackContext::getInstance()->port_type_size();
}

const char ** meta_jack_get_ports (jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags)
{
    return RecursiveJackContext::getInstance()->get_ports(client, port_name_pattern, type_name_pattern, flags);
}

jack_port_t * meta_jack_port_by_name (jack_client_t *client, const char *port_name)
{
    return RecursiveJackContext::getInstance()->port_by_name(client, port_name);
}

jack_port_t * meta_jack_port_by_id (jack_client_t *client, jack_port_id_t port_id)
{
    return RecursiveJackContext::getInstance()->port_by_id(client, port_id);
}

jack_nframes_t meta_jack_frames_since_cycle_start (const jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->frames_since_cycle_start(client);
}

jack_nframes_t meta_jack_frame_time (const jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->frame_time(client);
}

jack_nframes_t meta_jack_last_frame_time (const jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->last_frame_time(client);
}

jack_time_t meta_jack_frames_to_time(const jack_client_t *client, jack_nframes_t nframes)
{
    return RecursiveJackContext::getInstance()->frames_to_time(client, nframes);
}

jack_nframes_t meta_jack_time_to_frames(const jack_client_t *client, jack_time_t time)
{
    return RecursiveJackContext::getInstance()->time_to_frames(client, time);
}

jack_time_t meta_jack_get_time()
{
    return RecursiveJackContext::getInstance()->get_time();
}

void meta_jack_set_error_function (void (*func)(const char *))
{
    RecursiveJackContext::getInstance()->set_error_function(func);
}

void meta_jack_set_info_function (void (*func)(const char *))
{
    RecursiveJackContext::getInstance()->set_info_function(func);
}

void meta_jack_free(void* ptr)
{
    RecursiveJackContext::getInstance()->free(ptr);
}

int  meta_jack_release_timebase (jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->release_timebase(client);
}

int  meta_jack_set_sync_callback (jack_client_t *client, JackSyncCallback sync_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_sync_callback(client, sync_callback, arg);
}

int  meta_jack_set_sync_timeout (jack_client_t *client, jack_time_t timeout)
{
    return RecursiveJackContext::getInstance()->set_sync_timeout(client, timeout);
}

int  meta_jack_set_timebase_callback (jack_client_t *client, int conditional, JackTimebaseCallback timebase_callback, void *arg)
{
    return RecursiveJackContext::getInstance()->set_timebase_callback(client, conditional, timebase_callback, arg);
}

int  meta_jack_transport_locate (jack_client_t *client, jack_nframes_t frame)
{
    return RecursiveJackContext::getInstance()->transport_locate(client, frame);
}

jack_transport_state_t meta_jack_transport_query (const jack_client_t *client, jack_position_t *pos)
{
    return RecursiveJackContext::getInstance()->transport_query(client, pos);
}

jack_nframes_t meta_jack_get_current_transport_frame (const jack_client_t *client)
{
    return RecursiveJackContext::getInstance()->get_current_transport_frame(client);
}

int  meta_jack_transport_reposition (jack_client_t *client, jack_position_t *pos)
{
    return RecursiveJackContext::getInstance()->transport_reposition(client, pos);
}

void meta_jack_transport_start (jack_client_t *client)
{
    RecursiveJackContext::getInstance()->transport_start(client);
}

void meta_jack_transport_stop (jack_client_t *client)
{
    RecursiveJackContext::getInstance()->transport_stop(client);
}

void meta_jack_get_transport_info (jack_client_t *client, jack_transport_info_t *tinfo)
{
    RecursiveJackContext::getInstance()->get_transport_info(client, tinfo);
}

void meta_jack_set_transport_info (jack_client_t *client, jack_transport_info_t *tinfo)
{
    RecursiveJackContext::getInstance()->set_transport_info(client, tinfo);
}

#ifdef __cplusplus
}
#endif
