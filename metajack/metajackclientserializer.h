#ifndef JACKCLIENTSERIALIZER_H
#define JACKCLIENTSERIALIZER_H

#include <QDataStream>

class MetaJackClientSerializer
{
public:
    virtual void save(jack_client_t *client, QDataStream &stream) = 0;
    virtual jack_client_t * load(const QString &clientName, QDataStream &stream) = 0;
};

#endif // JACKCLIENTSERIALIZER_H
