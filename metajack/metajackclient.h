#ifndef METAJACKCLIENT_H
#define METAJACKCLIENT_H

#include <string>
#include <set>
#include <jack/jack.h>

class MetaJackPortBase;
class MetaJackPortNew;

class MetaJackClientBase {
public:
    MetaJackClientBase(const std::string &name);
    virtual ~MetaJackClientBase();
    const std::string & getName() const;
    void addPort(MetaJackPortBase *port);
    void removePort(MetaJackPortBase *port);
    void disconnect();
    bool isIndirectInputOf(MetaJackPortBase *port) const;
protected:
    std::set<MetaJackPortBase*> ports;
private:
    std::string name;
};

class MetaJackClientProcess : public MetaJackClientBase {
public:
    MetaJackClientProcess(const std::string &name);
    void setProcessCallback(JackProcessCallback processCallback, void *processCallbackArgument);
    bool process(std::set<MetaJackClientProcess*> &unprocessedClients, jack_nframes_t nframes);
private:
    JackProcessCallback processCallback;
    void * processCallbackArgument;
};

class MetaJackClientNew : public MetaJackClientBase {
public:
    MetaJackClientNew(const std::string &name);
    void setActive(bool active);
    bool isActive() const;
    MetaJackClientProcess * getProcessClient();
private:
    bool active;
    MetaJackClientProcess *twin;
};

class MetaJackContextNew;

class MetaJackDummyInputClientNew : public MetaJackClientNew {
public:
    MetaJackDummyInputClientNew(MetaJackContextNew *context, jack_port_t *wrapperAudioInputPort, jack_port_t *wrapperMidiInputPort);
private:
    MetaJackPortNew *audioOutputPort, *midiOutputPort;
    jack_port_t *wrapperAudioInputPort, *wrapperMidiInputPort;
    static int process(jack_nframes_t nframes, void *arg);
};

class MetaJackDummyOutputClientNew : public MetaJackClientNew {
public:
    MetaJackDummyOutputClientNew(MetaJackContextNew *context, jack_port_t *wrapperAudioOutputPort, jack_port_t *wrapperMidiOutputPort);
private:
    MetaJackPortNew *audioInputPort, *midiInputPort;
    jack_port_t *wrapperAudioOutputPort, *wrapperMidiOutputPort;
    static int process(jack_nframes_t nframes, void *arg);
};


#endif // METAJACKCLIENT_H
