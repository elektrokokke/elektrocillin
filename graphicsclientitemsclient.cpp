#include "graphicsclientitemsclient.h"

GraphicsClientItemsClient::GraphicsClientItemsClient(JackContextGraphicsScene *scene_) :
    JackClient("GraphicsClientItemsClient"),
    scene(scene_),
    clientStyle(3),
    portStyle(3),
    font("Helvetica", 12)
{
    setCallProcess(false);
    setEmitClientSignals(true);
}

GraphicsClientItemsClient::~GraphicsClientItemsClient()
{
    clear();
}

void GraphicsClientItemsClient::clear()
{
    // delete all client items:
    for (QMap<QString, GraphicsClientItem*>::iterator i = clientItems.begin(); i != clientItems.end(); i++) {
        delete i.value();
    }
    clientItems.clear();
}

void GraphicsClientItemsClient::onClientRegistered(const QString &clientName)
{
    // create a client item with that name:
    GraphicsClientItem *clientItem = new GraphicsClientItem(this, clientName, clientStyle, portStyle, font, 0, scene);
    clientItems.insert(clientName, clientItem);
}

void GraphicsClientItemsClient::onClientUnregistered(const QString &clientName)
{
    // delete the client item with the given name:
    delete clientItems.value(clientName, 0);
    clientItems.remove(clientName);
}
