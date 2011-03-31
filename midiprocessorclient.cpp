#include "midiprocessorclient.h"

MidiProcessorClient::MidiProcessorClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor_, unsigned int channels_) :
    AudioProcessorClient(clientName, audioProcessor),
    midiProcessor(midiProcessor_),
    midiInput(true),
    midiOutput(false),
    channels(channels_)
{
}

MidiProcessorClient::MidiProcessorClient(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, unsigned int channels_) :
    AudioProcessorClient(clientName, inputPortNames, outputPortNames),
    midiProcessor(0),
    midiInput(true),
    midiOutput(false),
    channels(channels_)
{
}

MidiProcessorClient::~MidiProcessorClient()
{
    close();
}

void MidiProcessorClient::activateMidiInput(bool active)
{
    midiInput = active;
}

void MidiProcessorClient::activateMidiOutput(bool active)
{
    midiOutput = active;
}

MidiProcessor * MidiProcessorClient::getMidiProcessor()
{
    return midiProcessor;
}

void MidiProcessorClient::setChannelMask(unsigned int channelMask)
{
    channels = channelMask;
}

unsigned int MidiProcessorClient::getChannelMask() const
{
    return channels;
}

bool MidiProcessorClient::init()
{
    return (!midiInput || (midiInputPort = registerMidiPort(QString("Midi in"), JackPortIsInput)))
            && (!midiOutput || (midiOutputPort = registerMidiPort(QString("Midi out"), JackPortIsOutput)))
            && AudioProcessorClient::init();
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
                processMidi(midiEvent, currentFrame);
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
    // only hand down the midi event if it has the right channel:
    if ((channels >> channel) & 1) {
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
        } else if (highNibble == 0x0A) {
           // aftertouch:
           unsigned char note = midiEvent.buffer[1];
           unsigned char pressure = midiEvent.buffer[2];
           processAfterTouch(channel, note, pressure, time);
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
        } else if (highNibble == 0x0D) {
            // channel pressure:
            unsigned char pressure = midiEvent.buffer[1];
            processChannelPressure(channel, pressure, time);
        }
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

void MidiProcessorClient::processAfterTouch(unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processAfterTouch(channel, noteNumber, pressure, time);
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

void MidiProcessorClient::processChannelPressure(unsigned char channel, unsigned char pressure, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processChannelPressure(channel, pressure, time);
}

void MidiProcessorClient::writeMidi(const MidiEvent &event, jack_nframes_t time)
{
    Q_ASSERT(midiOutput);
    jack_nframes_t lastFrameTime = getLastFrameTime();
    // adjust time relative to the beginning of this frame:
    if (time + nframes < lastFrameTime) {
        // if time is too early, this is in the buffer for too long, adjust time accordingly:
        time = 0;
    } else if (time >= lastFrameTime)  {
        time = nframes - 1;
    } else {
        time = time + nframes - lastFrameTime;
    }
    // write the event to the jack midi output buffer:
    jack_midi_event_write(midiOutputBuffer, time, event.buffer, event.size * sizeof(jack_midi_data_t));
}

void MidiProcessorClient::getMidiPortBuffer(jack_nframes_t nframes)
{
    if (midiInput) {
        // get MIDI input port buffer:
        midiInputBuffer = jack_port_get_buffer(midiInputPort, nframes);
        currentMidiEventIndex = 0;
        midiEventCount = jack_midi_get_event_count(midiInputBuffer);
    } else {
        midiEventCount = 0;
    }
    if (midiOutput) {
        // get MIDI output port buffer:
        midiOutputBuffer = jack_port_get_buffer(midiOutputPort, nframes);
        // clear it:
        jack_midi_clear_buffer(midiOutputBuffer);
    }
    this->nframes = nframes;
}
