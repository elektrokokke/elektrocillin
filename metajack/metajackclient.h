#ifndef METAJACKCLIENT_H
#define METAJACKCLIENT_H

#include <string>
#include <set>
#include <map>
#include <jack/types.h>
#include "sincfilter.h"

class MetaJackPortBase;
class MetaJackPort;

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

class MetaJackClient : public MetaJackClientBase {
public:
    MetaJackClient(const std::string &name);
    void setActive(bool active);
    bool isActive() const;
    MetaJackClientProcess * getProcessClient();
    std::set<MetaJackPortBase*> & getPorts();
private:
    bool active;
    MetaJackClientProcess *twin;
};

class MetaJackContext;
class JackContext;

class MetaJackInterfaceClient : public MetaJackClient {
public:
    MetaJackInterfaceClient(MetaJackContext *context, JackContext *wrapperInterface, int flags);
private:
    MetaJackContext *context;
    JackContext *wrapperInterface;
    std::map<MetaJackPort*, jack_port_t*> connectedPorts;
    std::map<MetaJackPort*, SincFilter> downsamplers;
    std::set<MetaJackPort*> freePorts;
    int wrapperAudioSuffix, wrapperMidiSuffix, audioSuffix, midiSuffix;
    static int process(jack_nframes_t nframes, void *arg);
    static void portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void *arg);
    std::string createPortName(const std::string &shortName, int suffix);
};

#endif // METAJACKCLIENT_H
