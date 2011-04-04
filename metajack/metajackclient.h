#ifndef METAJACKCLIENT_H
#define METAJACKCLIENT_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

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
    bool hasWrapperPorts() const;
private:
    MetaJackContext *context;
    JackContext *wrapperInterface;
    std::map<MetaJackPort*, jack_port_t*> connectedPorts;
    std::map<MetaJackPort*, SincFilter> downsamplers;
    std::set<MetaJackPort*> freePorts;
    int wrapperAudioSuffix, wrapperMidiSuffix, audioSuffix, midiSuffix;
    static int process(jack_nframes_t nframes, void *arg);
    static void portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void *arg);
    static std::string createPortName(const std::string &shortName, bool isInput, int suffix);
    void createNewPort(MetaJackPort *freePort, MetaJackPort *otherPort);
};

#endif // METAJACKCLIENT_H
