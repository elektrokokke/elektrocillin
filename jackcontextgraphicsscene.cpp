#include "jackcontextgraphicsscene.h"
#include "graphicsportconnectionitem.h"

JackContextGraphicsScene::JackContextGraphicsScene() :
    graphicsClientItemsClient(this)
{
    setBackgroundBrush(QBrush(QColor("lightsteelblue")));
}

void JackContextGraphicsScene::deleteClient(const QString &clientName)
{
    graphicsClientItemsClient.deleteClient(clientName);
}

void JackContextGraphicsScene::saveSession(QDataStream &stream)
{
    graphicsClientItemsClient.saveState(stream);
}

void JackContextGraphicsScene::loadSession(QDataStream &stream)
{
    graphicsClientItemsClient.loadState(stream);
}

void JackContextGraphicsScene::showAllInnerItems(bool visible)
{
    graphicsClientItemsClient.showAllInnerItems(visible);
}
