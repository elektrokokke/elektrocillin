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
