#ifndef REALJACKCONTEXT_H
#define REALJACKCONTEXT_H

/*
    Copyright 2011 Arne Jacobs

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jackcontext.h"
#include <jack/midiport.h>
#include <string>
#include <map>

class RealJackContext : public JackContext
{
public:
    RealJackContext();
    /**
      Returns a client handle iff it has been created by this instance.
      I.e., client handles of clients created by others can not be retrieved this way.
      */
    jack_client_t * client_by_name(const char *client_name);
    std::list<jack_client_t*> get_clients();
    const char * get_name() const;

    // Jack API methods:
    void get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr);
    const char * get_version_string();
    jack_client_t * client_open (const char *client_name, jack_options_t options, jack_status_t *, ...);
    int client_close (jack_client_t *client);
    int client_name_size ();
    char * get_client_name (jack_client_t *client);
    int activate (jack_client_t *client);
    int deactivate (jack_client_t *client);
    int get_client_pid (const char *);
    pthread_t client_thread_id (jack_client_t *client);
    int is_realtime (jack_client_t *client);
    int set_thread_init_callback (jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg);
    void on_shutdown (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg);
    void on_info_shutdown (jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg);
    int set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg);
    int set_freewheel_callback (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg);
    int set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg);
    int set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg);
    int set_client_registration_callback (jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg);
    int set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg);
    int set_port_connect_callback (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg);
    int set_port_rename_callback (jack_client_t *client, JackPortRenameCallback rename_callback, void *arg);
    int set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg);
    int set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg);
    int set_freewheel(jack_client_t *client, int onoff);
    int set_buffer_size (jack_client_t *client, jack_nframes_t nframes);
    jack_nframes_t get_sample_rate (jack_client_t *client);
    jack_nframes_t get_buffer_size (jack_client_t *client);
    float cpu_load (jack_client_t *client);
    jack_port_t * port_register (jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size);
    int port_unregister (jack_client_t *client, jack_port_t *port);
    void * port_get_buffer (jack_port_t *port, jack_nframes_t nframes);
    const char * port_name (const jack_port_t *port);
    const char * port_short_name (const jack_port_t *port);
    int port_flags (const jack_port_t *port);
    const char * port_type (const jack_port_t *port);
    int port_is_mine (const jack_client_t *client, const jack_port_t *port);
    int port_connected (const jack_port_t *port);
    int port_connected_to (const jack_port_t *port, const char *port_name);
    const char ** port_get_connections (const jack_port_t *port);
    const char ** port_get_all_connections (const jack_client_t *client, const jack_port_t *port);
    jack_nframes_t port_get_latency (jack_port_t *port);
    jack_nframes_t port_get_total_latency (jack_client_t *client, jack_port_t *port);
    void port_set_latency (jack_port_t *port, jack_nframes_t nframes);
    int recompute_total_latency (jack_client_t *client, jack_port_t *port);
    int recompute_total_latencies (jack_client_t *client);
    int port_set_name (jack_port_t *port, const char *port_name);
    int port_set_alias (jack_port_t *port, const char *alias);
    int port_unset_alias (jack_port_t *port, const char *alias);
    int port_get_aliases (const jack_port_t *port, char* const aliases[]);
    int port_request_monitor (jack_port_t *port, int onoff);
    int port_request_monitor_by_name (jack_client_t *client, const char *port_name, int onoff);
    int port_ensure_monitor (jack_port_t *port, int onoff);
    int port_monitoring_input (jack_port_t *port);
    int connect (jack_client_t *client, const char *source_port, const char *destination_port);
    int disconnect (jack_client_t *client, const char *source_port, const char *destination_port);
    int port_disconnect (jack_client_t *client, jack_port_t *port);
    int port_name_size();
    int port_type_size();
    const char ** get_ports (jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags);
    jack_port_t * port_by_name (jack_client_t *client, const char *port_name);
    jack_port_t * port_by_id (jack_client_t *client, jack_port_id_t port_id);
    jack_nframes_t frames_since_cycle_start (const jack_client_t *client);
    jack_nframes_t frame_time (const jack_client_t *client);
    jack_nframes_t last_frame_time (const jack_client_t *client);
    jack_time_t frames_to_time(const jack_client_t *client, jack_nframes_t nframes);
    jack_nframes_t time_to_frames(const jack_client_t *client, jack_time_t time);
    jack_time_t get_time();
    void set_error_function (void (*func)(const char *));
    void set_info_function (void (*func)(const char *));
    void free(void* ptr);
    // Jack transport API methods:
    int  release_timebase (jack_client_t *client);
    int  set_sync_callback (jack_client_t *client, JackSyncCallback sync_callback, void *arg);
    int  set_sync_timeout (jack_client_t *client, jack_time_t timeout);
    int  set_timebase_callback (jack_client_t *client, int conditional, JackTimebaseCallback timebase_callback, void *arg);
    int  transport_locate (jack_client_t *client, jack_nframes_t frame);
    jack_transport_state_t transport_query (const jack_client_t *client, jack_position_t *pos);
    jack_nframes_t get_current_transport_frame (const jack_client_t *client);
    int  transport_reposition (jack_client_t *client, jack_position_t *pos);
    void transport_start (jack_client_t *client);
    void transport_stop (jack_client_t *client);
    void get_transport_info (jack_client_t *client, jack_transport_info_t *tinfo);
    void set_transport_info (jack_client_t *client, jack_transport_info_t *tinfo);

    static jack_nframes_t midi_get_event_count(void* port_buffer);
    static int midi_event_get(jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index);
    static void midi_clear_buffer(void *port_buffer);
    static size_t midi_max_event_size(void* port_buffer);
    static jack_midi_data_t * midi_event_reserve(void *port_buffer, jack_nframes_t time, size_t data_size);
    static int midi_event_write(void *port_buffer, jack_nframes_t time, const jack_midi_data_t *data, size_t data_size);
    static jack_nframes_t midi_get_lost_event_count(void *port_buffer);

private:
    std::map<std::string, jack_client_t*> clients;
    std::string name;
};

#endif // REALJACKCONTEXT_H
