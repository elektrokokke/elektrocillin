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
