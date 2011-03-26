#ifndef METAJACKPORT_H
#define METAJACKPORT_H

#include <string>
#include <set>
#include <jack/types.h>

class MetaJackClientBase;
class MetaJackClientProcess;
class MetaJackClient;

class MetaJackPortBase {
public:
    MetaJackPortBase(jack_port_id_t id, const std::string &shortName, const std::string &type, int flags);
    virtual ~MetaJackPortBase();
    void setClient(MetaJackClientBase *client);
    const std::string & getShortName() const;
    void setShortName(const std::string &port_name);
    const std::string & getFullName() const;
    const std::string & getType() const;
    jack_port_id_t getId() const;
    int getFlags() const;
    bool isInput() const;
    size_t getConnectionCount() const;
    bool isConnectedTo(MetaJackPortBase *port) const;
    bool isIndirectInputOf(MetaJackPortBase *port) const;
    const char ** getConnections() const;
    void disconnect();
    void disconnect(MetaJackPortBase *port);
    void connect(MetaJackPortBase *port);
    bool belongsTo(const MetaJackClientBase *client) const;
    MetaJackClientBase * getClient();
    const std::set<MetaJackPortBase*> & getConnectedPorts();
protected:
    std::set<MetaJackPortBase*> connectedPorts;
private:
    MetaJackClientBase *client;
    jack_port_id_t id;
    std::string shortName, fullName, type;
    int flags;
};

class MetaJackPortProcess : public MetaJackPortBase {
public:
    MetaJackPortProcess(jack_port_id_t id, const std::string &shortName, const std::string &type, int flags, jack_nframes_t bufferSize);
    ~MetaJackPortProcess();
    void * getBuffer(jack_nframes_t nframes);
    void changeBufferSize(jack_nframes_t bufferSize);
    bool clearBuffer();
    bool mergeConnectedBuffers();
    bool process(std::set<MetaJackClientProcess*> &unprocessedClients, jack_nframes_t nframes);
private:
    size_t bufferSizeInBytes;
    char * buffer;
};

class MetaJackPort : public MetaJackPortBase {
public:
    MetaJackPort(MetaJackClient *client, jack_port_id_t id, const std::string &shortName, const std::string &type, int flags);
    void createProcessPort(jack_nframes_t bufferSize);
    MetaJackPortProcess * getProcessPort();
private:
    MetaJackPortProcess *twin;
};

#endif // METAJACKPORT_H
