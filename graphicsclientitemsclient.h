#ifndef GRAPHICSCLIENTITEMSCLIENT_H
#define GRAPHICSCLIENTITEMSCLIENT_H

#include "jackclient.h"
#include "jackcontextgraphicsscene.h"
#include "graphicsclientitem.h"
#include <QMap>

class GraphicsClientItemsClient : public JackClient
{
public:
    GraphicsClientItemsClient(JackContextGraphicsScene *scene);
    virtual ~GraphicsClientItemsClient();

    void clear();

protected slots:
    void onClientRegistered(const QString &clientName);
    void onClientUnregistered(const QString &clientName);
private:
    JackContextGraphicsScene *scene;
    QMap<QString, GraphicsClientItem*> clientItems;
    int clientStyle, portStyle;
    QFont font;
};

#endif // GRAPHICSCLIENTITEMSCLIENT_H
