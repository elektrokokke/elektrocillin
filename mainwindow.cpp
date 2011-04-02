/*
    Copyright 2011 Arne Jacobs

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jackcontextgraphicsscene.h"
#include "metajack/recursivejackcontext.h"

#include <QDebug>
#include <QFileDialog>
#include <QGLWidget>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings("settings.ini", QSettings::IniFormat)
{
    ui->setupUi(this);

    QList<JackClientFactory*> factories = JackClientSerializer::getInstance()->getFactories();
    for (QList<JackClientFactory*>::const_iterator i = factories.begin(); i != factories.end(); i++) {
        JackClientFactory *factory = *i;
        ui->menuNew_module->addAction(factory->getName(), this, SLOT(onActionCreateClient()));
    }

    scene = new JackContextGraphicsScene();
    QObject::connect(ui->actionPlay, SIGNAL(triggered()), scene, SLOT(play()));
    QObject::connect(ui->actionStop, SIGNAL(triggered()), scene, SLOT(stop()));
    QObject::connect(ui->actionRewind, SIGNAL(triggered()), scene, SLOT(rewind()));
    QObject::connect(ui->actionCreate_macro, SIGNAL(triggered()), scene, SLOT(createNewMacro()));
    QObject::connect(ui->actionEdit_macro, SIGNAL(triggered()), scene, SLOT(editSelectedMacro()));
    QObject::connect(ui->actionParent_level, SIGNAL(triggered()), scene, SLOT(exitCurrentMacro()));
    QObject::connect(scene, SIGNAL(selectionChanged()), this, SLOT(onSceneSelectionChanged()));
    QObject::connect(scene, SIGNAL(contextLevelChanged(int)), this, SLOT(onContextLevelChanged(int)));
    QObject::connect(scene, SIGNAL(messageChanged(QString)), ui->statusBar, SLOT(showMessage(QString)));

    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
//    ui->graphicsView->setViewport(new QGLWidget());
    ui->graphicsView->setScene(scene);
}

MainWindow::~MainWindow()
{
    delete ui->graphicsView->scene();
    delete ui;
}

void MainWindow::onActionCreateClient()
{
    scene->createNewModule(((QAction*)sender())->text());
}

void MainWindow::on_actionSave_session_triggered()
{
    // ask for the session file name:
    QString fileName = QFileDialog::getSaveFileName(this, "Save session", QString(), "Elektrocillin session files (*.elektrocillin)");
    if (!fileName.isNull()) {
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        QDataStream stream(&file);
        ((JackContextGraphicsScene*)ui->graphicsView->scene())->saveSession(stream);
    }
}

void MainWindow::on_actionLoad_session_triggered()
{
    // ask for the session file name:
    QString fileName = QFileDialog::getOpenFileName(this, "Load session", QString(), "Elektrocillin session files (*.elektrocillin)");
    if (!fileName.isNull()) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        QDataStream stream(&file);
        ((JackContextGraphicsScene*)ui->graphicsView->scene())->loadSession(stream);
    }
}

void MainWindow::on_actionReset_triggered()
{
    ui->graphicsView->resetTransform();
}

void MainWindow::on_actionShow_all_controls_triggered()
{
    ((JackContextGraphicsScene*)ui->graphicsView->scene())->showAllInnerItems();
    onSceneSelectionChanged();
}

void MainWindow::on_actionHide_all_controls_triggered()
{
    ((JackContextGraphicsScene*)ui->graphicsView->scene())->showAllInnerItems(false);
    onSceneSelectionChanged();
}

void MainWindow::onSceneSelectionChanged()
{
    // test if only one item is selected and if it is a macro:
    int selectedMacros = 0;
    int selectedModules = 0;
    int selectedVisibleModules = 0;
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
        if (GraphicsClientItem *clientItem = dynamic_cast<GraphicsClientItem*>(*i)) {
            if (clientItem->isMacroItem()) {
                selectedMacros++;
            } else if (clientItem->isModuleItem()) {
                selectedModules++;
                if (clientItem->isControlsVisible()) {
                    selectedVisibleModules++;
                }
            }
        }
    }
    ui->actionEdit_macro->setEnabled(selectedMacros == 1);
    ui->actionDelete_module->setEnabled(selectedModules);
    ui->actionDelete_module->setText(selectedModules <= 1 ? "Delete module" : "Delete modules");
    ui->actionDelete_macro->setEnabled(selectedMacros);
    ui->actionDelete_macro->setText(selectedMacros <= 1 ? "Delete macro" : "Delete macros");
    ui->actionShow_module_controls->setEnabled(selectedModules > selectedVisibleModules);
    ui->actionHide_module_controls->setEnabled(selectedVisibleModules);
    ui->actionZoom_to->setEnabled(selectedModules || selectedMacros);
}

void MainWindow::on_actionDelete_module_triggered()
{
    bool reallyDelete = true;
    if (ui->actionDelete_module->text() == "Delete modules") {
        reallyDelete = (QMessageBox::warning(this, "Delete multiple modules?", "Do you really want to delete all selected modules?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
    }
    if (reallyDelete) {
        // delete the selected modules (not selected macros!):
        QList<QGraphicsItem*> selectedItems = scene->selectedItems();
        for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
            if (GraphicsClientItem *clientItem = dynamic_cast<GraphicsClientItem*>(*i)) {
                if (clientItem->isModuleItem()) {
                    scene->deleteClient(clientItem->getClientName());
                }
            }
        }
    }
}

void MainWindow::on_actionDelete_macro_triggered()
{
    bool reallyDelete = true;
    if (ui->actionDelete_macro->text() == "Delete macros") {
        reallyDelete = (QMessageBox::warning(this, "Delete multiple macros?", "Do you really want to delete all selected macros?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
    }
    if (reallyDelete) {
        // delete the selected macros (not selected modules!):
        QList<QGraphicsItem*> selectedItems = scene->selectedItems();
        for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
            if (GraphicsClientItem *clientItem = dynamic_cast<GraphicsClientItem*>(*i)) {
                if (clientItem->isMacroItem()) {
                    scene->deleteClient(clientItem->getClientName());
                }
            }
        }
    }
}

void MainWindow::on_actionShow_module_controls_triggered()
{
    int selectedModules = 0;
    int selectedVisibleModules = 0;
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
        if (GraphicsClientItem *clientItem = dynamic_cast<GraphicsClientItem*>(*i)) {
            if (clientItem->isModuleItem()) {
                clientItem->setControlsVisible(true);
                selectedModules++;
                if (clientItem->isControlsVisible()) {
                    selectedVisibleModules++;
                }
            }
        }
    }
    ui->actionShow_module_controls->setEnabled(selectedModules > selectedVisibleModules);
    ui->actionHide_module_controls->setEnabled(selectedVisibleModules);
}

void MainWindow::on_actionHide_module_controls_triggered()
{
    int selectedModules = 0;
    int selectedVisibleModules = 0;
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
        if (GraphicsClientItem *clientItem = dynamic_cast<GraphicsClientItem*>(*i)) {
            if (clientItem->isModuleItem()) {
                clientItem->setControlsVisible(false);
                selectedModules++;
                if (clientItem->isControlsVisible()) {
                    selectedVisibleModules++;
                }
            }
        }
    }
    ui->actionShow_module_controls->setEnabled(selectedModules > selectedVisibleModules);
    ui->actionHide_module_controls->setEnabled(selectedVisibleModules);
}

void MainWindow::on_actionZoom_to_triggered()
{
    int selectedModules = 0;
    int selectedVisibleModules = 0;
    QRectF boundingRect;
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
        if (GraphicsClientItem *clientItem = dynamic_cast<GraphicsClientItem*>(*i)) {
            if (clientItem->isModuleItem()) {
                clientItem->setControlsVisible(true);
                selectedModules++;
                if (clientItem->isControlsVisible()) {
                    selectedVisibleModules++;
                }
            }
        }
        QRectF rect = ((*i)->childrenBoundingRect() | (*i)->boundingRect()).translated((*i)->pos());
        boundingRect |= rect;
    }
    ui->graphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);
    ui->actionShow_module_controls->setEnabled(selectedModules > selectedVisibleModules);
    ui->actionHide_module_controls->setEnabled(selectedVisibleModules);
}

void MainWindow::on_actionShow_all_triggered()
{
    QRectF boundingRect;
    QList<QGraphicsItem*> items = scene->items();
    for (QList<QGraphicsItem*>::iterator i = items.begin(); i != items.end(); i++) {
        if (dynamic_cast<GraphicsClientItem*>(*i)) {
            QRectF rect = ((*i)->childrenBoundingRect() | (*i)->boundingRect()).translated((*i)->pos());
            boundingRect |= rect;
        }
    }
    ui->graphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);
}

void MainWindow::onContextLevelChanged(int level)
{
    ui->actionParent_level->setEnabled(level);
    on_actionShow_all_triggered();
}

void MainWindow::on_actionNew_module_triggered()
{
    ui->menuNew_module->exec(QCursor::pos());
}
