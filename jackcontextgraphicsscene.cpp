#include "jackcontextgraphicsscene.h"
#include "graphicsportconnectionitem.h"
#include "metajack/recursivejackcontext.h"
#include "metajack/metajackcontext.h"
#include <QMessageBox>

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

bool JackContextGraphicsScene::editMacro(GraphicsClientItem *clientItem)
{
    if (clientItem->isMacroItem()) {
        // get the macro's wrapper client:
        JackContext *jackContext = RecursiveJackContext::getInstance()->getContextByClientName(clientItem->getClientName().toAscii().data());
        if (jackContext) {
            // make the macro's wrapper client the new context:
            RecursiveJackContext::getInstance()->pushExistingContext(jackContext);
            // change to that context:
            changeToCurrentContext();
            contextLevelChanged(RecursiveJackContext::getInstance()->getContextStackSize() - 1);
            return true;
        }
    }
    return false;
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

void JackContextGraphicsScene::exitCurrentMacro()
{
    if (RecursiveJackContext::getInstance()->getContextStackSize() > 1) {
        // test if the current context has any connections to the outside:
        bool deleteMacro = false;
        MetaJackContext *macroContext = (MetaJackContext*)RecursiveJackContext::getInstance()->getCurrentContext();
        if (!macroContext->hasWrapperPorts()) {
            // ask the user wether the macro should be left (which means it will be deleted):
            deleteMacro = (QMessageBox::warning(0, "Delete macro?", "The macro has no outside connections and will therefore be deleted if exited. Are you sure you want to delete the macro?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
            if (!deleteMacro) {
                return;
            }
        }
        // pop the current jack context:
        RecursiveJackContext::getInstance()->popContext();
        // change to that context:
        changeToCurrentContext();
        contextLevelChanged(RecursiveJackContext::getInstance()->getContextStackSize() - 1);
        // delete the macro if necessary:
        if (deleteMacro) {
            RecursiveJackContext::getInstance()->deleteContext(macroContext);
        }
    }
}

void JackContextGraphicsScene::editSelectedMacro()
{
    // determine if the item in focus is a macro:
    QList<QGraphicsItem*> items = selectedItems();
    for (QList<QGraphicsItem*>::iterator i = items.begin(); i != items.end(); i++) {
        if (GraphicsClientItem *clientItem = qgraphicsitem_cast<GraphicsClientItem*>(*i)) {
            if (editMacro(clientItem)) {
                return;
            }
        }
    }
}

void JackContextGraphicsScene::createNewMacro()
{
    // create a new wrapper client:
    RecursiveJackContext::getInstance()->pushNewContext("macro", 1);
    // change to that context:
    changeToCurrentContext();
    contextLevelChanged(RecursiveJackContext::getInstance()->getContextStackSize() - 1);
}

void JackContextGraphicsScene::createNewModule(QString factoryName)
{
    JackClientFactory *factory = JackClientSerializer::getInstance()->getFactoryByName(factoryName);
    if (factory) {
        factory->createClient(factory->getName())->activate();
    }
}
