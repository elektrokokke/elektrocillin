#include <jack/jack.h>
#include <stdarg.h>
#include <QString>
#include "metajackcontext.h"

#ifdef __cplusplus
extern "C"
{
#endif

//typedef struct _jack_port meta_jack_port_t;
//typedef struct _jack_client meta_jack_client_t;

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
  Note: the type meta_jack_client_t (_jack_client) has to be defined.
  A new instance has to be created in the current meta_jack_context.

  Status update is not yet implemented... as is any option except JackUseExactName.
  */
meta_jack_client_t * meta_jack_client_open (const char *client_name, jack_options_t options, jack_status_t *status, ...)
{
    //return jack_client_open(client_name, options, status);
    return MetaJackContext::instance->client_open(client_name, options);
}

int meta_jack_client_close (meta_jack_client_t *client)
{
    //return jack_client_close(client);
    return client->context->client_close(client);
}

// Note: no real need to reimplement this
int meta_jack_client_name_size (void)
{
    //return jack_client_name_size();
    return MetaJackContext::instance->client_get_name_size();
}

/*
  Note: the client name has to be stored somewhere in the
  client's meta_jack_context. Probably the meta_jack_client_t
  should be defined such that it includes that name.
  */
char * meta_jack_get_client_name (meta_jack_client_t *client)
{
    //return jack_get_client_name(client);
    return (char*)client->context->client_get_name(client).c_str();
}

/*
  Note: after calling this, the given client has to be
  considered in meta_jack's graph execution.
  Probably this could just mean to allow the creation
  of connections to this client's ports.
  */
int meta_jack_activate (meta_jack_client_t *client)
{
    //return jack_activate(client);
    return client->context->client_activate(client);
}

/*
  Note: remove all connections to this client and
  forbid making any further connections to it.
  Port buffer should be left as they are.
  */
int meta_jack_deactivate (meta_jack_client_t *client)
{
    //return jack_deactivate(client);
    return client->context->client_deactivate(client);
}

/*
  Note: return the PID of the meta_jack_context of
  this client, i.e. the PID of the real JACK client
  this meta client is capsuled in.
  */
int meta_jack_get_client_pid (const char *name)
{
    //return jack_get_client_pid(name);
    return MetaJackContext::instance->get_pid();
}

/*
  Note: return the thread which the meta_jack_context,
  i.e. the capsuling real JACK client, is running in.
  Sidenote: multithreading inside one meta_jack_context
  is not possible this way.
  */
pthread_t meta_jack_client_thread_id (meta_jack_client_t *client)
{
    //return jack_client_thread_id(client);
    return client->context->get_thread_id();
}

/*
  Note: get the given client's meta_jack_context. Then get
  the real JACK client it represents, and call jack_is_realtime
  with it.
  */
int meta_jack_is_realtime (meta_jack_client_t *client)
{
    //return jack_is_realtime(client);
    return client->context->is_realtime();
}

/*
  Note: not quite clear yet how to handle this...
  Probably just get the real client associated with the
  given client's meta_jack_context and then call the original
  implementation on it?
  */
int meta_jack_set_thread_init_callback (meta_jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    //return jack_set_thread_init_callback(client, thread_init_callback, arg);
    return client->context->set_thread_init_callback(client, thread_init_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
void meta_jack_on_shutdown (meta_jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg)
{
    //jack_on_shutdown(client, shutdown_callback, arg);
    client->context->set_shutdown_callback(client, shutdown_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
void meta_jack_on_info_shutdown (meta_jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    //jack_on_info_shutdown(client, shutdown_callback, arg);
    client->context->set_info_shutdown_callback(client, shutdown_callback, arg);
}

/*
  Note: the given callback definitely has to be stored and later called
  by meta_jack.
  */
int meta_jack_set_process_callback (meta_jack_client_t *client, JackProcessCallback process_callback, void *arg)
{
    //return jack_set_process_callback(client, process_callback, arg);
    return client->context->set_process_callback(client, process_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_freewheel_callback (meta_jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    //return jack_set_freewheel_callback(client, freewheel_callback, arg);
    return client->context->set_freewheel_callback(client, freewheel_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_buffer_size_callback (meta_jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    //return jack_set_buffer_size_callback(client, bufsize_callback, arg);
    return client->context->set_buffer_size_callback(client, bufsize_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_sample_rate_callback (meta_jack_client_t *client, JackSampleRateCallback srate_callback, void *arg)
{
    //return jack_set_sample_rate_callback(client, srate_callback, arg);
    return client->context->set_sample_rate_callback(client, srate_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  It should only be called when new clients register in the same
  meta_jack_context as the given client.
  */
int meta_jack_set_client_registration_callback (meta_jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    //return jack_set_client_registration_callback(client, registration_callback, arg);
    return client->context->set_client_registration_callback(client, registration_callback, arg);
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_port_registration_callback (meta_jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    //return jack_set_port_registration_callback(client, registration_callback, arg);
    return client->context->set_port_registration_callback(client, registration_callback, arg);
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_port_connect_callback (meta_jack_client_t *client, JackPortConnectCallback connect_callback, void *arg)
{
    //return jack_set_port_connect_callback(client, connect_callback, arg);
    return client->context->set_port_connect_callback(client, connect_callback, arg);
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_port_rename_callback (meta_jack_client_t *client, JackPortRenameCallback rename_callback, void *arg)
{
    //return jack_set_port_rename_callback(client, rename_callback, arg);
    return client->context->set_port_rename_callback(client, rename_callback, arg);
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_graph_order_callback (meta_jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg)
{
    //return jack_set_graph_order_callback(client, graph_callback, arg);
    return client->context->set_graph_order_callback(client, graph_callback, arg);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_xrun_callback (meta_jack_client_t *client, JackXRunCallback xrun_callback, void *arg)
{
    //return jack_set_xrun_callback(client, xrun_callback, arg);
    return client->context->set_xrun_callback(client, xrun_callback, arg);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
int meta_jack_set_freewheel(meta_jack_client_t* client, int onoff)
{
    //return jack_set_freewheel(client, onoff);
    return client->context->set_freewheel(onoff);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
int meta_jack_set_buffer_size (meta_jack_client_t *client, jack_nframes_t nframes)
{
    //return jack_set_buffer_size(client, nframes);
    return client->context->set_buffer_size(nframes);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_get_sample_rate (meta_jack_client_t *client)
{
    //return jack_get_sample_rate(client);
    return client->context->get_sample_rate();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_get_buffer_size (meta_jack_client_t *client)
{
    //return jack_get_buffer_size(client);
    return client->context->get_buffer_size();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
float meta_jack_cpu_load (meta_jack_client_t *client)
{
    //return jack_cpu_load(client);
    return client->context->get_cpu_load();
}

/*
  Note: the type meta_jack_port_t (_jack_port) has to be defined.
  A new instance has to be created in the current meta_jack_context.
  A buffer has to be created for the new port.
  */
meta_jack_port_t * meta_jack_port_register (meta_jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size)
{
    //return jack_port_register(client, port_name, port_type, flags, buffer_size);
    return client->context->port_register(client, port_name, port_type, flags, buffer_size);
}

/*
  Note: remove all connections from the port and free the buffer associated with the port.
  */
int meta_jack_port_unregister (meta_jack_client_t *client, meta_jack_port_t *port)
{
    //return jack_port_unregister(client, port);
    return client->context->port_unregister(port);
}

/*
  Note: get the buffer which is associated to the given port in its
  client's meta_jack_context.
  */
void * meta_jack_port_get_buffer (meta_jack_port_t *port, jack_nframes_t nframes)
{
    //return jack_port_get_buffer(port, nframes);
    return port->client->context->port_get_buffer(port, nframes);
}

/*
  Note: the port name has to be stored somewhere in the given
  port's client's meta_jack_context.
  Probably define the meta_jack_port_t type to store that name
  (or the short name, respectively, which means that the client
  name has to be appended).
  */
const char * meta_jack_port_name (const meta_jack_port_t *port)
{
    //return jack_port_name(port);
    return port->client->context->port_get_name(port).c_str();
}

/*
  Note: the port name has to be stored somewhere in the given
  port's client's meta_jack_context.
  Probably the meta_jack_port_t type should be defined to store that name.
  */
const char * meta_jack_port_short_name (const meta_jack_port_t *port)
{
    //return jack_port_short_name(port);
    return port->client->context->port_get_short_name(port).c_str();
}

/*
  Note: these flags should probably be stored in the yet-to-define
  meta_jack_port_t type.
  */
int meta_jack_port_flags (const meta_jack_port_t *port)
{
    //return jack_port_flags(port);
    return port->client->context->port_get_flags(port);
}

/*
  Note: the port type should probably be stored in the yet-to-define
  meta_jack_port_t type.
  */
const char * meta_jack_port_type (const meta_jack_port_t *port)
{
    //return jack_port_type(port);
    return port->client->context->port_get_type(port).c_str();
}

/*
  Note: not quite sure how to know what port type is associated with which
  port type id...
  */
jack_port_type_id_t meta_jack_port_type_id (const meta_jack_port_t *port)
{
    //return jack_port_type_id(port);
    return port->client->context->port_get_type_id(port);
}

/*
  Note: a port's client should probably be stored in the yet-to-define
  meta_jack_port_t type (which makes this test trivial).
  */
int meta_jack_port_is_mine (const meta_jack_client_t *client, const meta_jack_port_t *port)
{
    //return jack_port_is_mine(client, port);
    return client->context->port_is_mine(client, port);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_port_connected (const meta_jack_port_t *port)
{
    //return jack_port_connected(port);
    return port->client->context->port_get_connection_count(port);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_port_connected_to (const meta_jack_port_t *port, const char *port_name)
{
    //return jack_port_connected_to(port, port_name);
    return port->client->context->port_is_connected_to(port, port_name);
}

/*
  Note: meta_jack_free() should be defined accordingly (such that it can
  free memory returned by this function).
  Also: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
const char ** meta_jack_port_get_connections (const meta_jack_port_t *port)
{
    //return jack_port_get_connections(port);
    return port->client->context->port_get_connections(port);
}

/*
  Note: meta_jack_free() should be defined accordingly (such that it can
  free memory returned by this function).
  Also: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
const char ** meta_jack_port_get_all_connections (const meta_jack_client_t *client, const meta_jack_port_t *port)
{
    //return jack_port_get_all_connections(client, port);
    return client->context->port_get_all_connections(port);
}

/*
  Note: probably just return 0 here?
  */
jack_nframes_t meta_jack_port_get_latency (meta_jack_port_t *port)
{
    //return jack_port_get_latency(port);
    return port->client->context->port_get_latency(port);
}

/*
  Note: not quite sure how to implement this, as no port inside
  a meta_jack_context will have the JackPortIsTerminal flag set.
  Just return 0 or pass on to the real JACK server?
  */
jack_nframes_t meta_jack_port_get_total_latency (meta_jack_client_t *client, meta_jack_port_t *port)
{
    //return jack_port_get_total_latency(client, port);
    return client->context->port_get_total_latency(port);
}

/*
  Note: will probably not be needed in meta_jack, so just make this
  a no-op...
  */
void meta_jack_port_set_latency (meta_jack_port_t *port, jack_nframes_t nframes)
{
    //jack_port_set_latency(port, nframes);
}

/*
  Note: will probably not be needed in meta_jack, so just make this
  a no-op...
  */
int meta_jack_recompute_total_latency (meta_jack_client_t *client, meta_jack_port_t* port)
{
    //return jack_recompute_total_latency(client, port);
    return 1;
}

/*
  Note: will probably not be needed in meta_jack, so just make this
  a no-op...
  */
int meta_jack_recompute_total_latencies (meta_jack_client_t *client)
{
    //return jack_recompute_total_latencies(client);
    return 1;
}

/*
  Note: this depends on how meta_jack_port_t (_jack_port) is implemented.
  The callback handed to meta_jack_set_port_rename_callback should be called here if the port's
  client has registered such a callback.
  */
int meta_jack_port_set_name (meta_jack_port_t *port, const char *port_name)
{
    //return jack_port_set_name(port, port_name);
    return port->client->context->port_set_name(port, port_name);
}

/*
  Note: probably have a map of such aliases in a meta_jack_context.
  Assure that each port has two aliases at most. Also assure that
  no two ports have the same alias.
  */
int meta_jack_port_set_alias (meta_jack_port_t *port, const char *alias)
{
    //return jack_port_set_alias(port, alias);
    return port->client->context->port_set_alias(port, alias);
}

/*
  Note: probably have a map of such aliases in a meta_jack_context.
  */
int meta_jack_port_unset_alias (meta_jack_port_t *port, const char *alias)
{
    //return jack_port_unset_alias(port, alias);
    return port->client->context->port_unset_alias(port, alias);
}

/*
  Note: probably have a map of such aliases in a meta_jack_context.
  */
int meta_jack_port_get_aliases (const meta_jack_port_t *port, char* const aliases[2])
{
    //return jack_port_get_aliases(port, aliases);
    return port->client->context->port_get_aliases(port, aliases);
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_request_monitor (meta_jack_port_t *port, int onoff)
{
    //return jack_port_request_monitor(port, onoff);
    return 1;
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_request_monitor_by_name (meta_jack_client_t *client, const char *port_name, int onoff)
{
    //return jack_port_request_monitor_by_name(client, port_name, onoff);
    return 1;
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_ensure_monitor (meta_jack_port_t *port, int onoff)
{
    //return jack_port_ensure_monitor(port, onoff);
    return 1;
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_monitoring_input (meta_jack_port_t *port)
{
    //return jack_port_monitoring_input(port);
    return 0;
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_connect (meta_jack_client_t *client, const char *source_port, const char *destination_port)
{
    //return jack_connect(client, source_port, destination_port);
    return client->context->port_connect(source_port, destination_port);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_disconnect (meta_jack_client_t *client, const char *source_port, const char *destination_port)
{
    //return jack_disconnect(client, source_port, destination_port);
    return client->context->port_disconnect(source_port, destination_port);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_port_disconnect (meta_jack_client_t *client, meta_jack_port_t *port)
{
    //return jack_port_disconnect(client, port);
    return client->context->port_disconnect(client, port);
}

// Note: no need to reimplement this
int meta_jack_port_name_size(void)
{
    //return jack_port_name_size();
    return MetaJackContext::instance->port_get_name_size();
}

// Note: no need to reimplement this
int meta_jack_port_type_size(void)
{
    //return jack_port_type_size();
    return MetaJackContext::instance->port_get_type_size();
}

/*
  Note: what regular expression implementation could be used here?
  meta_jack_free() should be defined accordingly (such that it can
  free memory returned by this function).
  Also: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
const char ** meta_jack_get_ports (meta_jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags)
{
    //return jack_get_ports(client, port_name_pattern, type_name_pattern, flags);
    return client->context->get_ports(port_name_pattern, type_name_pattern, flags);
}

/*
  Note: look through the given client's ports to see which one has the given name.
  */
meta_jack_port_t * meta_jack_port_by_name (meta_jack_client_t *client, const char *port_name)
{
    //return jack_port_by_name(client, port_name);
    return client->context->get_port_by_name(port_name);
}

/*
  Note: look through the given client's ports to see which one has the given id.
  */
meta_jack_port_t * meta_jack_port_by_id (meta_jack_client_t *client, jack_port_id_t port_id)
{
    //return jack_port_by_id(client, port_id);
    return client->context->get_port_by_id(port_id);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_frames_since_cycle_start (const meta_jack_client_t *client)
{
    //return jack_frames_since_cycle_start(client);
    return client->context->get_frames_since_cycle_start();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_frame_time (const meta_jack_client_t *client)
{
    //return jack_frame_time(client);
    return client->context->get_frame_time();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_last_frame_time (const meta_jack_client_t *client)
{
    //return jack_last_frame_time(client);
    return client->context->get_last_frame_time();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_time_t meta_jack_frames_to_time(const meta_jack_client_t *client, jack_nframes_t nframes)
{
    //return jack_frames_to_time(client, nframes);
    return client->context->convert_frames_to_time(nframes);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_time_to_frames(const meta_jack_client_t *client, jack_time_t time)
{
    //return jack_time_to_frames(client, time);
    return client->context->convert_time_to_frames(time);
}

// Note: probably no need to reimplement this
jack_time_t meta_jack_get_time()
{
    //return jack_get_time();
    return MetaJackContext::instance->get_time();
}

/*
  Note: should clients capsuled in a meta_jack_context receive error messages?
  */
void meta_jack_set_error_function (void (*func)(const char *))
{
    //jack_set_error_function(func);
}

/*
  Note: should clients capsuled in a meta_jack_context receive error messages?
  */
void meta_jack_set_info_function (void (*func)(const char *))
{
    //jack_set_info_function(func);
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
    //jack_free(ptr);
    MetaJackContext::instance->free(ptr);
}

#ifdef __cplusplus
}
#endif
