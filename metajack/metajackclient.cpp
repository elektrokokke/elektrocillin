#include "metajackclient.h"
#include "metajackport.h"
#include "metajackcontext.h"
#include <jack/midiport.h>
#include <sstream>
#include <cassert>

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

MetaJackClient::MetaJackClient(const std::string &name) :
    MetaJackClientBase(name),
    active(false),
    twin(new MetaJackClientProcess(name))
{}

void MetaJackClient::setActive(bool active)
{
    this->active = active;
}

bool MetaJackClient::isActive() const
{
    return active;
}

MetaJackClientProcess * MetaJackClient::getProcessClient()
{
    return twin;
}

std::set<MetaJackPortBase*> & MetaJackClient::getPorts()
{
    return ports;
}

MetaJackInterfaceClient::MetaJackInterfaceClient(MetaJackContext *context_, JackContext *wrapperInterface_, int flags) :
    MetaJackClient((flags & JackPortIsOutput) ? "system_in" : "system_out"),
    context(context_),
    wrapperInterface(wrapperInterface_),
    wrapperAudioSuffix(1),
    wrapperMidiSuffix(1),
    audioSuffix(1),
    midiSuffix(1)
{
    assert((flags & JackPortIsInput) || (flags & JackPortIsOutput));
    freePorts.insert(context->registerPort(this, createPortName("audio", audioSuffix++), JACK_DEFAULT_AUDIO_TYPE, flags, 0));
    freePorts.insert(context->registerPort(this, createPortName("midi", midiSuffix++), JACK_DEFAULT_MIDI_TYPE, flags, 0));
    context->setProcessCallback(this, process, this);
    context->portConnectCallbackHandler[this] = std::make_pair(portConnectCallback, this);
}

int MetaJackInterfaceClient::process(jack_nframes_t nframes, void *arg)
{
    // the purpose of the dummy input client is to make the wrapper client inputs available to clients inside the wrapper
    MetaJackInterfaceClient *me = (MetaJackInterfaceClient*)arg;
    unsigned int oversampling = me->context->getOversampling();
    for (std::map<MetaJackPort*, jack_port_t*>::iterator i = me->connectedPorts.begin(); i != me->connectedPorts.end(); i++) {
        // for each connected port, copy from the wrapper client's port to the corresponding internal port:
        MetaJackPort *port = i->first;
        jack_port_t *wrapperPort = i->second;
        if (port && wrapperPort) {
            if (port->getType() == JACK_DEFAULT_AUDIO_TYPE) {
                // copy audio:
                jack_default_audio_sample_t *wrapperAudioBuffer = (jack_default_audio_sample_t*)me->wrapperInterface->port_get_buffer(wrapperPort, nframes / oversampling);
                jack_default_audio_sample_t *audioBuffer = (jack_default_audio_sample_t*)me->context->getPortBuffer(port, nframes);
                if (port->isInput()) {
                    // downsampling:
                    SincFilter &downsampler = me->downsamplers.at(port);
                    if (oversampling > 1) {
                        for (jack_nframes_t i = 0, wrapperi = 0; i < nframes; i += oversampling, wrapperi++) {
                            // apply a sinc filter for bandlimiting:
                            for (jack_nframes_t j = 0; j < oversampling; j++) {
                                downsampler.feed(audioBuffer[i + j]);
                            }
                            wrapperAudioBuffer[wrapperi] = downsampler.process();
                        }
                    } else {
                        for (jack_nframes_t i = 0; i < nframes; i++) {
                            wrapperAudioBuffer[i] = audioBuffer[i];
                        }
                    }
                } else {
                    // upsampling:
                    for (jack_nframes_t i = 0, wrapperi = 0; i < nframes; i += oversampling, wrapperi++) {
                        // currently just copying of samples:
                        for (jack_nframes_t j = 0; j < oversampling; j++) {
                            audioBuffer[i + j] = wrapperAudioBuffer[wrapperi];
                        }
                    }
                }
            } else if (port->getType() == JACK_DEFAULT_MIDI_TYPE) {
                // copy midi:
                void *wrapperMidiBuffer = me->wrapperInterface->port_get_buffer(wrapperPort, nframes / oversampling);
                void *midiBuffer = me->context->getPortBuffer(port, nframes);
                // important: check wether the midi port is from the real jack server or from a MetaJackContext:
                bool metaJackMidi = dynamic_cast<MetaJackContext*>(me->wrapperInterface);
                if (port->isInput()) {
                    if (metaJackMidi) {
                        MetaJackContext::midi_clear_buffer(wrapperMidiBuffer);
                    } else {
                        jack_midi_clear_buffer(wrapperMidiBuffer);
                    }
                    jack_nframes_t midiEventCount = MetaJackContext::midi_get_event_count(midiBuffer);
                    for (jack_nframes_t i = 0; i < midiEventCount; i++) {
                        jack_midi_event_t event;
                        MetaJackContext::midi_event_get(&event, midiBuffer, i);
                        // downsampling:
                        event.time /= oversampling;
                        if (metaJackMidi) {
                            MetaJackContext::midi_event_write(wrapperMidiBuffer, event.time, event.buffer, event.size);
                        } else {
                            jack_midi_event_write(wrapperMidiBuffer, event.time, event.buffer, event.size);
                        }
                    }
                } else {
                    MetaJackContext::midi_clear_buffer(midiBuffer);
                    jack_nframes_t midiEventCount = (metaJackMidi ? MetaJackContext::midi_get_event_count(wrapperMidiBuffer) : jack_midi_get_event_count(wrapperMidiBuffer));
                    for (jack_nframes_t i = 0; i < midiEventCount; i++) {
                        jack_midi_event_t event;
                        if (metaJackMidi) {
                            MetaJackContext::midi_event_get(&event, wrapperMidiBuffer, i);
                        } else {
                            jack_midi_event_get(&event, wrapperMidiBuffer, i);
                        }
                        // upsampling:
                        event.time *= oversampling;
                        MetaJackContext::midi_event_write(midiBuffer, event.time, event.buffer, event.size);
                    }
                }
            }
        }
    }
    return 0;
}

void MetaJackInterfaceClient::portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void *arg)
{
    MetaJackInterfaceClient *me = (MetaJackInterfaceClient*)arg;
    MetaJackPort *portA = me->context->getPortById(a);
    MetaJackPort *portB = me->context->getPortById(b);
    if (connect) {
        MetaJackPort *freePort = 0;
        MetaJackPort *otherPort = 0;
        if (me->freePorts.find(portA) != me->freePorts.end()) {
            freePort = portA;
            otherPort = portB;
        } else if (me->freePorts.find(portB) != me->freePorts.end()) {
            freePort = portB;
            otherPort = portA;
        }
        // if anything connects to one of the free output ports, put it into the connected ports list and create a new free one:
        if (freePort) {
            // one of the unconnected output port has been connected, create a corresponding real jack input port:
            std::string wrapperPortName = me->createPortName(otherPort->getShortName(), freePort->getType() == JACK_DEFAULT_AUDIO_TYPE ? me->wrapperAudioSuffix++ : me->wrapperMidiSuffix++);
            jack_port_t *wrapperPort = me->context->createWrapperPort(wrapperPortName, freePort->getType(), freePort->isInput() ? JackPortIsOutput : JackPortIsInput);
            me->connectedPorts[freePort] = wrapperPort;
            if (freePort->getType() == JACK_DEFAULT_AUDIO_TYPE) {
                unsigned int oversampling = me->context->getOversampling();
                me->downsamplers.insert(std::make_pair(freePort, SincFilter(oversampling * 3, 0.5, oversampling)));
            }
            me->freePorts.erase(freePort);
            // create a new free port:
            std::string newPortName = freePort->getType() == JACK_DEFAULT_AUDIO_TYPE ? me->createPortName("audio", me->audioSuffix++) : me->createPortName("midi", me->midiSuffix++);
            me->freePorts.insert(me->context->registerPort(me, newPortName, freePort->getType(), freePort->getFlags(), 0));
        }
    } else {
        // if a port is freed by disconnecting, we make it the new free port and delete the current free port:
        // TODO
    }
}

std::string MetaJackInterfaceClient::createPortName(const std::string &shortName, int suffix)
{
    std::stringstream portNameStream;
    portNameStream << shortName << " " << suffix;
    return portNameStream.str();
}
