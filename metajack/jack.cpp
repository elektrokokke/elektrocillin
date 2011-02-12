#include <jack/jack.h>
#include <stdarg.h>
#include <QString>

#ifdef __cplusplus
extern "C"
{
#endif

/*
  All notes regard what is to be done to make meta_jack work,
  not the current state, which just passes on calls to the original JACK API.
  */

// Note: no need to reimplement this
void meta_jack_get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr)
{
    jack_get_version(major_ptr, minor_ptr, micro_ptr, proto_ptr);
}

// Note: no need to reimplement this
const char * meta_jack_get_version_string()
{
    return jack_get_version_string();
}

/*
  Note: the type jack_client_t (_jack_client) has to be defined.
  A new instance has to be created in the current meta_jack_context.
  */
jack_client_t * meta_jack_client_open (const char *client_name, jack_options_t options, jack_status_t *status, ...)
{
    // for now, just ignore the optional JackServerName parameter:
//    QString clientName = QString("meta_%1").arg(client_name);
//    return jack_client_open(clientName.toAscii().data(), options, status);
    return jack_client_open(client_name, options, status);
}

int meta_jack_client_close (jack_client_t *client)
{
    return jack_client_close(client);
}

// Note: no real need to reimplement this
int meta_jack_client_name_size (void)
{
    return jack_client_name_size();
}

/*
  Note: the client name has to be stored somewhere in the
  client's meta_jack_context. Probably the jack_client_t
  should be defined such that it includes that name.
  */
char * meta_jack_get_client_name (jack_client_t *client)
{
    return jack_get_client_name(client);
}

/*
  Note: after calling this, the given client has to be
  considered in meta_jack's graph execution.
  Probably this could just mean to allow the creation
  of connections to this client's ports.
  */
int meta_jack_activate (jack_client_t *client)
{
    return jack_activate(client);
}

/*
  Note: remove all connections to this client and
  forbid making any further connections to it.
  Port buffer should be left as they are.
  */
int meta_jack_deactivate (jack_client_t *client)
{
    return jack_deactivate(client);
}

/*
  Note: return the PID of the meta_jack_context of
  this client, i.e. the PID of the real JACK client
  this meta client is capsuled in.
  */
int meta_jack_get_client_pid (const char *name)
{
    return jack_get_client_pid(name);
}

/*
  Note: return the thread which the meta_jack_context,
  i.e. the capsuling real JACK client, is running in.
  Sidenote: multithreading inside one meta_jack_context
  is not possible this way.
  */
pthread_t meta_jack_client_thread_id (jack_client_t *client)
{
    return jack_client_thread_id(client);
}

/*
  Note: get the given client's meta_jack_context. Then get
  the real JACK client it represents, and call jack_is_realtime
  with it.
  */
int meta_jack_is_realtime (jack_client_t *client)
{
    return jack_is_realtime(client);
}

/*
  Note: not quite clear yet how to handle this...
  Probably just get the real client associated with the
  given client's meta_jack_context and then call the original
  implementation on it?
  */
int meta_jack_set_thread_init_callback (jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    return jack_set_thread_init_callback(client, thread_init_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
void meta_jack_on_shutdown (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg)
{
    jack_on_shutdown(client, shutdown_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
void meta_jack_on_info_shutdown (jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    jack_on_info_shutdown(client, shutdown_callback, arg);
}

/*
  Note: the given callback definitely has to be stored and later called
  by meta_jack.
  */
int meta_jack_set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg)
{
    return jack_set_process_callback(client, process_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_freewheel_callback (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    return jack_set_freewheel_callback(client, freewheel_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    return jack_set_buffer_size_callback(client, bufsize_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg)
{
    return jack_set_sample_rate_callback(client, srate_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  It should only be called when new clients register in the same
  meta_jack_context as the given client.
  */
int meta_jack_set_client_registration_callback (jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    return jack_set_client_registration_callback(client, registration_callback, arg);
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    return jack_set_port_registration_callback(client, registration_callback, arg);
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_port_connect_callback (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg)
{
    return jack_set_port_connect_callback(client, connect_callback, arg);
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_port_rename_callback (jack_client_t *client, JackPortRenameCallback rename_callback, void *arg)
{
    return jack_set_port_rename_callback(client, rename_callback, arg);
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg)
{
    return jack_set_graph_order_callback(client, graph_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg)
{
    return jack_set_xrun_callback(client, xrun_callback, arg);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
int meta_jack_set_freewheel(jack_client_t* client, int onoff)
{
    return jack_set_freewheel(client, onoff);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
int meta_jack_set_buffer_size (jack_client_t *client, jack_nframes_t nframes)
{
    return jack_set_buffer_size(client, nframes);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_get_sample_rate (jack_client_t *client)
{
    return jack_get_sample_rate(client);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_get_buffer_size (jack_client_t *client)
{
    return jack_get_buffer_size(client);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
float meta_jack_cpu_load (jack_client_t *client)
{
    return jack_cpu_load(client);
}

/*
  Note: the type jack_port_t (_jack_port) has to be defined.
  A new instance has to be created in the current meta_jack_context.
  A buffer has to be created for the new port.
  */
jack_port_t * meta_jack_port_register (jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size)
{
    return jack_port_register(client, port_name, port_type, flags, buffer_size);
}

/*
  Note: remove all connections from the port and free the buffer associated with the port.
  */
int meta_jack_port_unregister (jack_client_t *client, jack_port_t *port)
{
    return jack_port_unregister(client, port);
}

/*
  Note: get the buffer which is associated to the given port in its
  client's meta_jack_context.
  */
void * meta_jack_port_get_buffer (jack_port_t *port, jack_nframes_t nframes)
{
    return jack_port_get_buffer(port, nframes);
}

/*
  Note: the port name has to be stored somewhere in the given
  port's client's meta_jack_context.
  Probably define the jack_port_t type to store that name
  (or the short name, respectively, which means that the client
  name has to be appended).
  */
const char * meta_jack_port_name (const jack_port_t *port)
{
    return jack_port_name(port);
}

/*
  Note: the port name has to be stored somewhere in the given
  port's client's meta_jack_context.
  Probably the jack_port_t type should be defined to store that name.
  */
const char * meta_jack_port_short_name (const jack_port_t *port)
{
    return jack_port_short_name(port);
}

/*
  Note: these flags should probably be stored in the yet-to-define
  jack_port_t type.
  */
int meta_jack_port_flags (const jack_port_t *port)
{
    return jack_port_flags(port);
}

/*
  Note: the port type should probably be stored in the yet-to-define
  jack_port_t type.
  */
const char * meta_jack_port_type (const jack_port_t *port)
{
    return jack_port_type(port);
}

/*
  Note: not quite sure how to know what port type is associated with which
  port type id...
  */
jack_port_type_id_t meta_jack_port_type_id (const jack_port_t *port)
{
    return jack_port_type_id(port);
}

/*
  Note: a port's client should probably be stored in the yet-to-define
  jack_port_t type (which makes this test trivial).
  */
int meta_jack_port_is_mine (const jack_client_t *client, const jack_port_t *port)
{
    return jack_port_is_mine(client, port);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_port_connected (const jack_port_t *port)
{
    return jack_port_connected(port);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_port_connected_to (const jack_port_t *port, const char *port_name)
{
    return jack_port_connected_to(port, port_name);
}

/*
  Note: meta_jack_free() should be defined accordingly (such that it can
  free memory returned by this function).
  Also: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
const char ** meta_jack_port_get_connections (const jack_port_t *port)
{
    return jack_port_get_connections(port);
}

/*
  Note: meta_jack_free() should be defined accordingly (such that it can
  free memory returned by this function).
  Also: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
const char ** meta_jack_port_get_all_connections (const jack_client_t *client, const jack_port_t *port)
{
    return jack_port_get_all_connections(client, port);
}

/*
  Note: probably just return 0 here?
  */
jack_nframes_t meta_jack_port_get_latency (jack_port_t *port)
{
    return jack_port_get_latency(port);
}

/*
  Note: not quite sure how to implement this, as no port inside
  a meta_jack_context will have the JackPortIsTerminal flag set.
  Just return 0 or pass on to the real JACK server?
  */
jack_nframes_t meta_jack_port_get_total_latency (jack_client_t *client, jack_port_t *port)
{
    return jack_port_get_total_latency(client, port);
}

/*
  Note: will probably not be needed in meta_jack, so just make this
  a no-op...
  */
void meta_jack_port_set_latency (jack_port_t *port, jack_nframes_t nframes)
{
    jack_port_set_latency(port, nframes);
}

/*
  Note: will probably not be needed in meta_jack, so just make this
  a no-op...
  */
int meta_jack_recompute_total_latency (jack_client_t *client, jack_port_t* port)
{
    return jack_recompute_total_latency(client, port);
}

/*
  Note: will probably not be needed in meta_jack, so just make this
  a no-op...
  */
int meta_jack_recompute_total_latencies (jack_client_t *client)
{
    return jack_recompute_total_latencies(client);
}

/*
  Note: this depends on how jack_port_t (_jack_port) is implemented.
  The callback handed to meta_jack_set_port_rename_callback should be called here if the port's
  client has registered such a callback.
  */
int meta_jack_port_set_name (jack_port_t *port, const char *port_name)
{
    return jack_port_set_name(port, port_name);
}

/*
  Note: probably have a map of such aliases in a meta_jack_context.
  Assure that each port has two aliases at most. Also assure that
  no two ports have the same alias.
  */
int meta_jack_port_set_alias (jack_port_t *port, const char *alias)
{
    return jack_port_set_alias(port, alias);
}

/*
  Note: probably have a map of such aliases in a meta_jack_context.
  */
int meta_jack_port_unset_alias (jack_port_t *port, const char *alias)
{
    return jack_port_unset_alias(port, alias);
}

/*
  Note: probably have a map of such aliases in a meta_jack_context.
  */
int meta_jack_port_get_aliases (const jack_port_t *port, char* const aliases[2])
{
    return jack_port_get_aliases(port, aliases);
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_request_monitor (jack_port_t *port, int onoff)
{
    return jack_port_request_monitor(port, onoff);
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_request_monitor_by_name (jack_client_t *client, const char *port_name, int onoff)
{
    return jack_port_request_monitor_by_name(client, port_name, onoff);
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_ensure_monitor (jack_port_t *port, int onoff)
{
    return jack_port_ensure_monitor(port, onoff);
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_monitoring_input (jack_port_t *port)
{
    return jack_port_monitoring_input(port);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_connect (jack_client_t *client, const char *source_port, const char *destination_port)
{
//    QString port1 = QString("meta_%1").arg(source_port);
//    QString port2 = QString("meta_%1").arg(destination_port);
//    return jack_connect(client, port1.toAscii().data(), port2.toAscii().data());
    return jack_connect(client, source_port, destination_port);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_disconnect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return jack_disconnect(client, source_port, destination_port);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_port_disconnect (jack_client_t *client, jack_port_t *port)
{
    return jack_port_disconnect(client, port);
}

// Note: no need to reimplement this
int meta_jack_port_name_size(void)
{
    return jack_port_name_size();
}

// Note: no need to reimplement this
int meta_jack_port_type_size(void)
{
    return jack_port_type_size();
}

/*
  Note: what regular expression implementation could be used here?
  meta_jack_free() should be defined accordingly (such that it can
  free memory returned by this function).
  Also: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
const char ** meta_jack_get_ports (jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags)
{
    return jack_get_ports(client, port_name_pattern, type_name_pattern, flags);
}

/*
  Note: look through the given client's ports to see which one has the given name.
  */
jack_port_t * meta_jack_port_by_name (jack_client_t *client, const char *port_name)
{
    return jack_port_by_name(client, port_name);
}

/*
  Note: look through the given client's ports to see which one has the given id.
  */
jack_port_t * meta_jack_port_by_id (jack_client_t *client, jack_port_id_t port_id)
{
    return jack_port_by_id(client, port_id);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_frames_since_cycle_start (const jack_client_t *client)
{
    return jack_frames_since_cycle_start(client);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_frame_time (const jack_client_t *client)
{
    return jack_frame_time(client);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_last_frame_time (const jack_client_t *client)
{
    return jack_last_frame_time(client);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_time_t meta_jack_frames_to_time(const jack_client_t *client, jack_nframes_t nframes)
{
    return jack_frames_to_time(client, nframes);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_time_to_frames(const jack_client_t *client, jack_time_t time)
{
    return jack_time_to_frames(client, time);
}

// Note: probably no need to reimplement this
jack_time_t meta_jack_get_time()
{
    return jack_get_time();
}

/*
  Note: should clients capsuled in a meta_jack_context receive error messages?
  */
void meta_jack_set_error_function (void (*func)(const char *))
{
    jack_set_error_function(func);
}

/*
  Note: should clients capsuled in a meta_jack_context receive error messages?
  */
void meta_jack_set_info_function (void (*func)(const char *))
{
    jack_set_info_function(func);
}

/*
  Note: this has to be able to free memory pointed to by pointers
  returned by the following functions:
  - meta_jack_port_get_connections()
  - meta_jack_port_get_all_connections()
  - meta_jack_get_ports
  */
void meta_jack_free(void* ptr)
{
    jack_free(ptr);
}

#ifdef __cplusplus
}
#endif
