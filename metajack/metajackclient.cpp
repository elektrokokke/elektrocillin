#include "metajackclient.h"
#include "metajackport.h"
#include "metajackcontextnew.h"

MetaJackClientBase::MetaJackClientBase(const std::string &name_) :
    name(name_)
{}

MetaJackClientBase::~MetaJackClientBase()
{
    for (; ports.size(); ) {
        MetaJackPortBase* port = *ports.begin();
        delete port;
    }
}

const std::string & MetaJackClientBase::getName() const
{
    return name;
}

void MetaJackClientBase::addPort(MetaJackPortBase *port)
{
    ports.insert(port);
}

void MetaJackClientBase::removePort(MetaJackPortBase *port)
{
    ports.erase(port);
}

void MetaJackClientBase::disconnect()
{
    for (std::set<MetaJackPortBase*>::iterator i = ports.begin(); i != ports.end(); i++) {
        MetaJackPortBase* port = *i;
        port->disconnect();
    }
}

bool MetaJackClientBase::isIndirectInputOf(MetaJackPortBase *port) const
{
    if (!port->isInput() && port->belongsTo(this)) {
        return true;
    }
    for (std::set<MetaJackPortBase*>::const_iterator i = ports.begin(); i != ports.end(); i++) {
        MetaJackPortBase *connectedPort = *i;
        if (!connectedPort->isInput() && connectedPort->isIndirectInputOf(port)) {
            return true;
        }
    }
    return false;
}

MetaJackClientProcess::MetaJackClientProcess(const std::string &name) :
    MetaJackClientBase(name),
    processCallback(0),
    processCallbackArgument(0)
{}

void MetaJackClientProcess::setProcessCallback(JackProcessCallback processCallback, void *processCallbackArgument)
{
    this->processCallback = processCallback;
    this->processCallbackArgument = processCallbackArgument;
}

bool MetaJackClientProcess::process(std::set<MetaJackClientProcess*> &unprocessedClients, jack_nframes_t nframes)
{
    // recursively process all clients that are connected to this client's input ports first:
    for (std::set<MetaJackPortBase*>::iterator i = ports.begin(); i != ports.end(); i++) {
        MetaJackPortProcess *port = (MetaJackPortProcess*)*i;
        // make sure all the client's buffers are initialized:
        port->getBuffer(nframes);
        // process all other clients that are connected to one of our inputs first:
        if (port->isInput()) {
            if (!port->process(unprocessedClients, nframes)) {
                return false;
            }
        }
    }
    // process this client:
    if (processCallback) {
        int errorCode = processCallback(nframes, processCallbackArgument);
        if (errorCode) {
            return false;
        }
    }
    // processing succeeded:
    unprocessedClients.erase(this);
    return true;
}

MetaJackClientNew::MetaJackClientNew(const std::string &name) :
    MetaJackClientBase(name),
    active(false),
    twin(new MetaJackClientProcess(name))
{}

void MetaJackClientNew::setActive(bool active)
{
    this->active = active;
}

bool MetaJackClientNew::isActive() const
{
    return active;
}

MetaJackClientProcess * MetaJackClientNew::getProcessClient()
{
    return twin;
}

MetaJackDummyInputClientNew::MetaJackDummyInputClientNew(MetaJackContextNew *context, jack_port_t *wrapperAudioInputPort_, jack_port_t *wrapperMidiInputPort_) :
    MetaJackClientNew("system_in"),
    wrapperAudioInputPort(wrapperAudioInputPort_),
    wrapperMidiInputPort(wrapperMidiInputPort_)
{
    audioOutputPort = context->registerPort(this, "capture_audio", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    midiOutputPort = context->registerPort(this, "capture_midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
    context->setProcessCallback(this, process, this);

}

MetaJackDummyOutputClientNew::MetaJackDummyOutputClientNew(MetaJackContextNew *context, jack_port_t *wrapperAudioOutputPort_, jack_port_t *wrapperMidiOutputPort_) :
    MetaJackClientNew("system_out"),
    wrapperAudioOutputPort(wrapperAudioOutputPort_),
    wrapperMidiOutputPort(wrapperMidiOutputPort_)
{
    audioInputPort = context->registerPort(this, "playback_audio", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    midiInputPort = context->registerPort(this, "playback_midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    context->setProcessCallback(this, process, this);

}

int MetaJackDummyInputClientNew::process(jack_nframes_t nframes, void *arg)
{
    // the purpose of the dummy input client is to make the wrapper client inputs available to clients inside the wrapper
    MetaJackDummyInputClientNew *me = (MetaJackDummyInputClientNew*)arg;
   // copy audio:
    jack_default_audio_sample_t *wrapperAudioInputBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(me->wrapperAudioInputPort, nframes);
    jack_default_audio_sample_t *audioOutputBuffer = (jack_default_audio_sample_t*)me->audioOutputPort->getProcessPort()->getBuffer(nframes);
    for (jack_nframes_t i = 0; i < nframes; i++) {
        audioOutputBuffer[i] = wrapperAudioInputBuffer[i];
    }
    // copy midi:
    void *wrapperMidiInputBuffer = jack_port_get_buffer(me->wrapperMidiInputPort, nframes);
    void *midiOutputBuffer = me->midiOutputPort->getProcessPort()->getBuffer(nframes);
    MetaJackContextNew::midi_clear_buffer(midiOutputBuffer);
    jack_nframes_t midiEventCount = jack_midi_get_event_count(wrapperMidiInputBuffer);
    for (jack_nframes_t i = 0; i < midiEventCount; i++) {
        jack_midi_event_t event;
        jack_midi_event_get(&event, wrapperMidiInputBuffer, i);
        MetaJackContextNew::midi_event_write(midiOutputBuffer, event.time, event.buffer, event.size);
    }
    return 0;
}

int MetaJackDummyOutputClientNew::process(jack_nframes_t nframes, void *arg)
{
    // the purpose of the dummy output client is to make the wrapper client outputs available to clients inside the wrapper
    MetaJackDummyOutputClientNew *me = (MetaJackDummyOutputClientNew*)arg;
    // copy audio:
    jack_default_audio_sample_t *audioInputBuffer = (jack_default_audio_sample_t*)me->audioInputPort->getProcessPort()->getBuffer(nframes);
    jack_default_audio_sample_t *wrapperAudioOutputBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(me->wrapperAudioOutputPort, nframes);
    for (jack_nframes_t i = 0; i < nframes; i++) {
        wrapperAudioOutputBuffer[i] = audioInputBuffer[i];
    }
    // copy midi:
    void *midiInputBuffer = me->midiInputPort->getProcessPort()->getBuffer(nframes);
    void *wrapperMidiOutputBuffer = jack_port_get_buffer(me->wrapperMidiOutputPort, nframes);
    jack_midi_clear_buffer(wrapperMidiOutputBuffer);
    jack_nframes_t midiEventCount = MetaJackContextNew::midi_get_event_count(midiInputBuffer);
    for (jack_nframes_t i = 0; i < midiEventCount; i++) {
        jack_midi_event_t event;
        MetaJackContextNew::midi_event_get(&event, midiInputBuffer, i);
        jack_midi_event_write(wrapperMidiOutputBuffer, event.time, event.buffer, event.size);
    }
    return 0;
}
