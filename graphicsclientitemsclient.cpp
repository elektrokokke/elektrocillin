#include "graphicsclientitemsclient.h"
#include "metajack/recursivejackcontext.h"

GraphicsClientItemsClient::GraphicsClientItemsClient(QGraphicsScene *scene_) :
    JackClient("GraphicsClientItemsClient"),
    scene(scene_),
    clientStyle(3),
    portStyle(3),
    font("Helvetica", 12)
{
    setCallProcess(false);
    setEmitPortSignals(true);
    setEmitClientSignals(true);
    activate();
    // get all clients and create visual representations for them:
    QStringList clientNames = getClients();
    for (int i = 0; i < clientNames.size(); i++) {
        onClientRegistered(clientNames[i]);
    }
    // make sure we're notified when clients are registered or unregistered:
    QObject::connect(this, SIGNAL(clientRegistered(QString)), this, SLOT(onClientRegistered(QString)));
    QObject::connect(this, SIGNAL(clientUnregistered(QString)), this, SLOT(onClientUnregistered(QString)));
}

GraphicsClientItemsClient::~GraphicsClientItemsClient()
{
    clear();
}

void GraphicsClientItemsClient::saveState(QDataStream &stream)
{
    RecursiveJackContext::getInstance()->saveCurrentContext(stream, JackClientSerializer::getInstance());
}

void GraphicsClientItemsClient::loadState(QDataStream &stream)
{
    // first delete all current clients:
    deleteClients();
    RecursiveJackContext::getInstance()->loadCurrentContext(stream, JackClientSerializer::getInstance());
}

void GraphicsClientItemsClient::setClientStyle(int clientStyle)
{
    this->clientStyle = clientStyle;
}

void GraphicsClientItemsClient::setPortStyle(int portStyle)
{
    this->portStyle = portStyle;
}


void GraphicsClientItemsClient::clear()
{
    // delete all client graphics items:
    for (QMap<QString, GraphicsClientItem*>::iterator i = clientItems.begin(); i != clientItems.end(); i++) {
        delete i.value();
    }
    clientItems.clear();
}

void GraphicsClientItemsClient::deleteClient(const QString &clientName)
{
    jack_client_t *client = meta_jack_client_by_name(clientName.toAscii().data());
    JackClient * jackClient;
    if (client && (jackClient = JackClientSerializer::getInstance()->getClient(client))) {
        delete jackClient;
    }
}

void GraphicsClientItemsClient::deleteClients()
{
    // delete all clients:
    QStringList clientNames = clientItems.keys();
    for (int i = 0; i < clientNames.size(); i++) {
        QString clientName = clientNames[i];
        deleteClient(clientName);
    }
}

void GraphicsClientItemsClient::showAllInnerItems(bool visible)
{
    for (QMap<QString, GraphicsClientItem*>::iterator i = clientItems.begin(); i != clientItems.end(); i++) {
        GraphicsClientItem *clientItem = i.value();
        if (clientItem) {
            clientItem->showInnerItem(visible);
        }
    }
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
