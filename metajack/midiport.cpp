#include "recursivejackcontext.h"

#ifdef __cplusplus
extern "C"
{
#endif

jack_nframes_t meta_jack_midi_get_event_count(void* port_buffer)
{
    return RecursiveJackContext::midi_get_event_count(port_buffer);
}

int meta_jack_midi_event_get(jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index)
{
    return RecursiveJackContext::midi_event_get(event, port_buffer, event_index);
}

void meta_jack_midi_clear_buffer(void *port_buffer)
{
    RecursiveJackContext::midi_clear_buffer(port_buffer);
}

size_t meta_jack_midi_max_event_size(void* port_buffer)
{
    return RecursiveJackContext::midi_max_event_size(port_buffer);
}

jack_midi_data_t* meta_jack_midi_event_reserve(void *port_buffer, jack_nframes_t  time, size_t data_size)
{
    return RecursiveJackContext::midi_event_reserve(port_buffer, time, data_size);
}

int meta_jack_midi_event_write(void *port_buffer, jack_nframes_t time, const jack_midi_data_t *data, size_t data_size)
{
    return RecursiveJackContext::midi_event_write(port_buffer, time, data, data_size);
}

jack_nframes_t meta_jack_midi_get_lost_event_count(void *port_buffer)
{
    return RecursiveJackContext::midi_get_lost_event_count(port_buffer);
}

#ifdef __cplusplus
}
#endif
