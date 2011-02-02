#ifndef NOTETRIGGEREDCLIENT_H
#define NOTETRIGGEREDCLIENT_H

#include "sampledclient.h"
#include "notetriggered.h"

class NoteTriggeredClient : public SampledClient
{
public:
    NoteTriggeredClient(const QString &clientName, NoteTriggered *noteTriggered);
    virtual ~NoteTriggeredClient();

protected:
    void processMidi(const jack_midi_event_t &midiEvent);

    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value);
    virtual void processPitchBend(unsigned char channel, unsigned int value);

private:
    NoteTriggered *noteTriggered;
};

#endif // NOTETRIGGEREDCLIENT_H
