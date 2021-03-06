/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jackcontextgraphicsscene.h"
#include "graphicsportconnectionitem.h"
#include "metajack/recursivejackcontext.h"
#include "metajack/metajackcontext.h"
#include <QMessageBox>
#include <QApplication>
#include <QFile>

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

void JackContextGraphicsScene::loadMacro(const QString &fileName)
{
    macroFileName = fileName;
    waitForMacroPosition = true;
    // set a special cursor to show the user what we are waiting for:
    QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
    // also send a message for the user to read:
    messageChanged("Left-click anywhere in the canvas to place the macro to load. Click with any other mouse button to abort macro creation.");
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
        if (GraphicsClientItem *clientItem = dynamic_cast<GraphicsClientItem*>(*i)) {
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
    graphicsClientItemsClient->setClientItemPositionByName(context->getWrapperClientName(), pos);
    // is there anything to load?
    if (!macroFileName.isNull()) {
        // load into the new context:
        QFile file(macroFileName);
        file.open(QIODevice::ReadOnly);
        QDataStream stream(&file);
        // load system client positions and save them in settings:
        QString contextName = context->get_name();
        QMap<QString, QPointF> clientItemPositionMap;
        stream >> clientItemPositionMap;
        for (QMap<QString, QPointF>::iterator i = clientItemPositionMap.begin(); i != clientItemPositionMap.end(); i++) {
            graphicsClientItemsClient->getSettings()->setValue("position/" + contextName + "/" + i.key(), i.value().toPoint());
        }
        // load the context clients:
        RecursiveJackContext::getInstance()->loadCurrentContext(stream, JackClientSerializer::getInstance());
        RecursiveJackContext::getInstance()->popContext();
    } else {
        // change to the new context:
        changeToCurrentContext();
        contextLevelChanged(RecursiveJackContext::getInstance()->getContextStackSize() - 1);
    }
}

void JackContextGraphicsScene::createNewModule(QString factoryName, QPointF pos)
{
    JackClientFactory *factory = JackClientSerializer::getInstance()->getFactoryByName(factoryName);
    if (factory) {
        clearSelection();
        JackClient *client = factory->createClient(factory->getName());
        client->setClientItemPosition(pos);
        client->activate();
    }
}
