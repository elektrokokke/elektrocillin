#include "midiprocessorclient.h"

MidiProcessorClient::MidiProcessorClient(const QString &clientName, MidiProcessor *midiProcessor_) :
    AudioProcessorClient(clientName, midiProcessor_),
    midiProcessor(midiProcessor_)
{
}

MidiProcessorClient::MidiProcessorClient(const QString &clientName, int nrOfInputs, int nrOfOutputs) :
    AudioProcessorClient(clientName, nrOfInputs, nrOfOutputs),
    midiProcessor(0)
{
}

MidiProcessorClient::~MidiProcessorClient()
{
    close();
}

MidiProcessor * MidiProcessorClient::getMidiProcessor()
{
    return (MidiProcessor*)getAudioProcessor();
}

bool MidiProcessorClient::init()
{
    return AudioProcessorClient::init() && (midiInputPort = registerMidiPort(QString("midi in"), JackPortIsInput));
}

bool MidiProcessorClient::process(jack_nframes_t nframes)
{
    // get audio port buffers:
    getPortBuffers(nframes);
    getMidiPortBuffer(nframes);
    // process all MIDI events:
    processMidi(0, nframes);
    return true;
}

void MidiProcessorClient::processMidi(jack_nframes_t start, jack_nframes_t end)
{
    jack_nframes_t lastFrameTime = getLastFrameTime();
    // only process the MIDI events between start and end:
    for (jack_nframes_t currentFrame = start; currentFrame < end; ) {
        // get the next midi event, if there is any:
        if (currentMidiEventIndex < midiEventCount) {
            jack_midi_event_t jackMidiEvent;
            jack_midi_event_get(&jackMidiEvent, midiInputBuffer, currentMidiEventIndex);
            if ((jackMidiEvent.size <= 3) && (jackMidiEvent.time < end)) {
                // produce audio until the event happens:
                processAudio(currentFrame, jackMidiEvent.time);
                currentFrame = jackMidiEvent.time;
                currentMidiEventIndex++;
                // copy the midi event to our own structure:
                MidiEvent midiEvent;
                midiEvent.size = jackMidiEvent.size;
                memcpy(midiEvent.buffer, jackMidiEvent.buffer, jackMidiEvent.size * sizeof(jack_midi_data_t));
                processMidi(midiEvent, currentFrame + lastFrameTime);
            } else {
                // produce audio until the end of the buffer:
                processAudio(currentFrame, end);
                currentFrame = end;
            }
        } else {
            // produce audio until the end of the buffer:
            processAudio(currentFrame, end);
            currentFrame = end;
        }
    }
}

void MidiProcessorClient::processMidi(const MidiProcessorClient::MidiEvent &midiEvent, jack_nframes_t time)
{
    // interpret the midi event:
    unsigned char statusByte = midiEvent.buffer[0];
    unsigned char highNibble = statusByte >> 4;
    unsigned char channel = statusByte & 0x0F;
    if (highNibble == 0x08) {
        unsigned char noteNumber = midiEvent.buffer[1];
        unsigned char velocity = midiEvent.buffer[2];
        processNoteOff(channel, noteNumber, velocity, time);
    } else if (highNibble == 0x09) {
        unsigned char noteNumber = midiEvent.buffer[1];
        unsigned char velocity = midiEvent.buffer[2];
        if (velocity) {
            // note on event:
            processNoteOn(channel, noteNumber, velocity, time);
        } else {
            // note off event:
            processNoteOff(channel, noteNumber, velocity, time);
        }
    } else if (highNibble == 0x0B) {
        // control change:
        unsigned char controller = midiEvent.buffer[1];
        unsigned char value = midiEvent.buffer[2];
        processController(channel, controller, value, time);
    } else if (highNibble == 0x0E) {
        // pitch wheel:
        unsigned char low = midiEvent.buffer[1];
        unsigned char high = midiEvent.buffer[2];
        unsigned int pitch = (high << 7) + low;
        processPitchBend(channel, pitch, time);
    }
}

void MidiProcessorClient::processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processNoteOn(channel, noteNumber, velocity, time);
}

void MidiProcessorClient::processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processNoteOff(channel, noteNumber, velocity, time);
}

void MidiProcessorClient::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processController(channel, controller, value, time);
}

void MidiProcessorClient::processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processPitchBend(channel, value, time);
}

void MidiProcessorClient::getMidiPortBuffer(jack_nframes_t nframes)
{
    // get midi port buffer:
    midiInputBuffer = jack_port_get_buffer(midiInputPort, nframes);
    currentMidiEventIndex = 0;
    midiEventCount = jack_midi_get_event_count(midiInputBuffer);
}
