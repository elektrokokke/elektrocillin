#include "notetriggeredclient.h"

NoteTriggeredClient::NoteTriggeredClient(const QString &clientName, NoteTriggered *noteTriggered_) :
    SampledClient(clientName, noteTriggered_, true),
    noteTriggered(noteTriggered_)
{
}

NoteTriggeredClient::~NoteTriggeredClient()
{
    close();
}

void NoteTriggeredClient::processMidi(const jack_midi_event_t &midiEvent)
{
    // interpret the midi event:
    unsigned char statusByte = midiEvent.buffer[0];
    unsigned char highNibble = statusByte >> 4;
    unsigned char channel = statusByte & 0x0F;
    if (highNibble == 0x08) {
        unsigned char noteNumber = midiEvent.buffer[1];
        unsigned char velocity = midiEvent.buffer[2];
        processNoteOff(channel, noteNumber, velocity);
    } else if (highNibble == 0x09) {
        unsigned char noteNumber = midiEvent.buffer[1];
        unsigned char velocity = midiEvent.buffer[2];
        if (velocity) {
            // note on event:
            processNoteOn(channel, noteNumber, velocity);
        } else {
            // note off event:
            processNoteOff(channel, noteNumber, velocity);
        }
    } else if (highNibble == 0x0B) {
        // control change:
        unsigned char controller = midiEvent.buffer[1];
        unsigned char value = midiEvent.buffer[2];
        processController(channel, controller, value);
    } else if (highNibble == 0x0E) {
        // pitch wheel:
        unsigned char low = midiEvent.buffer[1];
        unsigned char high = midiEvent.buffer[2];
        unsigned int pitch = (high << 7) + low;
        processPitchBend(channel, pitch);
    }
}

void NoteTriggeredClient::processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity)
{
    noteTriggered->noteOn(channel, noteNumber, velocity);
}

void NoteTriggeredClient::processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity)
{
    noteTriggered->noteOff(channel, noteNumber, velocity);
}

void NoteTriggeredClient::processController(unsigned char channel, unsigned char controller, unsigned char value)
{
    noteTriggered->controller(channel, controller, value);
}

void NoteTriggeredClient::processPitchBend(unsigned char channel, unsigned int value)
{
    noteTriggered->pitchBend(channel, value);
}
