#include "jackcontextgraphicsscene.h"
#include "graphicsportconnectionitem.h"

JackContextGraphicsScene::JackContextGraphicsScene() :
    graphicsClientItemsClient(new GraphicsClientItemsClient(this))
{
    setBackgroundBrush(QBrush(QColor("lightsteelblue")));
}

JackContextGraphicsScene::~JackContextGraphicsScene()
{
    delete graphicsClientItemsClient;
}

void JackContextGraphicsScene::saveSession(QDataStream &stream)
{
    graphicsClientItemsClient->saveState(stream);
}

void JackContextGraphicsScene::loadSession(QDataStream &stream)
{
    graphicsClientItemsClient->loadState(stream);
}

void JackContextGraphicsScene::changeToCurrentContext()
{
    delete graphicsClientItemsClient;
    clear();
    graphicsClientItemsClient = new GraphicsClientItemsClient(this);
}

void JackContextGraphicsScene::deleteClient(const QString &clientName)
{
    graphicsClientItemsClient->deleteClient(clientName);
}

void JackContextGraphicsScene::showAllInnerItems(bool visible)
{
    graphicsClientItemsClient->showAllInnerItems(visible);
}

void JackContextGraphicsScene::play()
{
    graphicsClientItemsClient->play();
}

void JackContextGraphicsScene::stop()
{
    graphicsClientItemsClient->stop();
}

void JackContextGraphicsScene::rewind()
{
    graphicsClientItemsClient->rewind();
}
