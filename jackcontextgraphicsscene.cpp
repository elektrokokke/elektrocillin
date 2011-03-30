#include "jackcontextgraphicsscene.h"
#include "graphicsportconnectionitem.h"
#include "metajack/recursivejackcontext.h"
#include "metajack/metajackcontext.h"
#include <QMessageBox>
#include <QApplication>

JackContextGraphicsScene::JackContextGraphicsScene() :
    graphicsClientItemsClient(new GraphicsClientItemsClient(this)),
    waitForMacroPosition(false),
    waitForModulePosition(false)
{
    setBackgroundBrush(QBrush(QColor("lightsteelblue")));

    QGraphicsRectItem *dummy = new QGraphicsRectItem(-5000, -5000, 10000, 10000, 0, this);
    dummy->setVisible(false);
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
    QGraphicsRectItem *dummy = new QGraphicsRectItem(-5000, -5000, 10000, 10000, 0, this);
    dummy->setVisible(false);
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
    if (!waitForMacroPosition) {
        waitForMacroPosition = true;
        // set a special cursor to show the user what we are waiting for:
        QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
        // also send a message for the user to read:
        messageChanged("Left-click anywhere in the canvas to place the new macro. Click with any other mouse button to abort macro creation.");
    }
}

void JackContextGraphicsScene::createNewModule(QString factoryName)
{
    if (!waitForModulePosition) {
        waitForModulePosition = true;
        this->factoryName = factoryName;
        // set a special cursor to show the user what we are waiting for:
        QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
        // also send a message for the user to read:
        messageChanged("Left-click anywhere in the canvas to place the new module. Click with any other mouse button to abort module creation.");
    }
}

void JackContextGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    if (waitForMacroPosition || waitForModulePosition) {
        mouseEvent->accept();
        if (mouseEvent->button() == Qt::LeftButton) {
            if (waitForMacroPosition) {
                createNewMacro(mouseEvent->scenePos());
            } else if (waitForModulePosition) {
                createNewModule(factoryName, mouseEvent->scenePos());
            }
        }
        waitForMacroPosition = waitForModulePosition = false;
        // reset cursor and message to normal:
        QApplication::restoreOverrideCursor();
        messageChanged(QString());
    } else {
        QGraphicsScene::mousePressEvent(mouseEvent);
    }
}

void JackContextGraphicsScene::createNewMacro(QPointF pos)
{
    // create a new wrapper client:
    MetaJackContext *context = (MetaJackContext*)RecursiveJackContext::getInstance()->pushNewContext("macro", 1);
    QString contextName = context->get_name();
    graphicsClientItemsClient->setClientItemPositionByName(context->getWrapperClientName(), pos);
    // change to that context:
    changeToCurrentContext();
    contextLevelChanged(RecursiveJackContext::getInstance()->getContextStackSize() - 1);
}

void JackContextGraphicsScene::createNewModule(QString factoryName, QPointF pos)
{
    JackClientFactory *factory = JackClientSerializer::getInstance()->getFactoryByName(factoryName);
    if (factory) {
        JackClient *client = factory->createClient(factory->getName());
        client->setClientItemPosition(pos);
        client->activate();
    }
}
