#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jackcontextgraphicsscene.h"
#include "metajack/recursivejackcontext.h"

#include <QDebug>
#include <QFileDialog>
#include <QGLWidget>
#include <QInputDialog>
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
        JackClientFactoryAction *action = new JackClientFactoryAction(factory, ui->menuCreate_client);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionCreateClient()));
        ui->menuNew_module->addAction(action);
    }

    scene = new JackContextGraphicsScene();
    QObject::connect(ui->actionPlay, SIGNAL(triggered()), scene, SLOT(play()));
    QObject::connect(ui->actionStop, SIGNAL(triggered()), scene, SLOT(stop()));
    QObject::connect(ui->actionRewind, SIGNAL(triggered()), scene, SLOT(rewind()));
    QObject::connect(scene, SIGNAL(selectionChanged()), this, SLOT(onSceneSelectionChanged()));

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
    if (JackClientFactoryAction *action = qobject_cast<JackClientFactoryAction*>(sender())) {
        (action->getFactory()->createClient(action->getFactory()->getName()))->activate();
//        ((JackContextGraphicsScene*)ui->graphicsView->scene())->addClient(action->getFactory()->createClient(action->getFactory()->getName()));
    }
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

void MainWindow::on_actionParent_level_triggered()
{
    // pop the current jack context:
    RecursiveJackContext::getInstance()->popContext();
    // change to that context:
    scene->changeToCurrentContext();
    ui->actionParent_level->setEnabled(RecursiveJackContext::getInstance()->getContextStackSize() > 1);
}

void MainWindow::on_actionEdit_macro_triggered()
{
    // determine if the item in focus is a macro:
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
        if (GraphicsClientItem *clientItem = qgraphicsitem_cast<GraphicsClientItem*>(*i)) {
            if (clientItem->isMacroItem()) {
                // get the macro's wrapper client:
                JackContext *jackContext = RecursiveJackContext::getInstance()->getContextByClientName(clientItem->getClientName().toAscii().data());
                if (jackContext) {
                    // make the macro's wrapper client the new context:
                    RecursiveJackContext::getInstance()->pushExistingContext(jackContext);
                    // change to that context:
                    scene->changeToCurrentContext();
                    ui->actionParent_level->setEnabled(RecursiveJackContext::getInstance()->getContextStackSize() > 1);
                }
                return;
            }
        }
    }
}

void MainWindow::on_actionCreate_macro_triggered()
{
    // ask the user for the macro name:
    QString macroName = QInputDialog::getText(this, "New macro", "Please enter the macro name:");
    if (!macroName.isNull()) {
        // get the oversampling factor:
        int oversampling = QInputDialog::getInt(this, "Oversampling", "Please enter the oversampling factor", 1, 1, 16);
        // create a new wrapper client:
        RecursiveJackContext::getInstance()->pushNewContext(macroName.toAscii().data(), oversampling);
        // change to that context:
        scene->changeToCurrentContext();
        ui->actionParent_level->setEnabled(RecursiveJackContext::getInstance()->getContextStackSize() > 1);
    }
}

void MainWindow::on_actionShow_all_controls_triggered()
{
    ((JackContextGraphicsScene*)ui->graphicsView->scene())->showAllInnerItems();
}

void MainWindow::on_actionHide_all_controls_triggered()
{
    ((JackContextGraphicsScene*)ui->graphicsView->scene())->showAllInnerItems(false);
}

void MainWindow::onSceneSelectionChanged()
{
    // test if only one item is selected and if it is a macro:
    int selectedMacros = 0;
    int selectedModules = 0;
    int selectedVisibleModules = 0;
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
        if (GraphicsClientItem *clientItem = qgraphicsitem_cast<GraphicsClientItem*>(*i)) {
            if (clientItem->isMacroItem()) {
                selectedMacros++;
            } else if (clientItem->isModuleItem()) {
                selectedModules++;
                if (clientItem->isInnerItemVisible()) {
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
        reallyDelete = (QMessageBox::question(this, "Delete multiple modules?", "Do you really want to delete all selected modules?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
    }
    if (reallyDelete) {
        // delete the selected modules (not selected macros!):
        QList<QGraphicsItem*> selectedItems = scene->selectedItems();
        for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
            if (GraphicsClientItem *clientItem = qgraphicsitem_cast<GraphicsClientItem*>(*i)) {
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
        reallyDelete = (QMessageBox::question(this, "Delete multiple macros?", "Do you really want to delete all selected macros?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
    }
    if (reallyDelete) {
        // delete the selected macros (not selected modules!):
        QList<QGraphicsItem*> selectedItems = scene->selectedItems();
        for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
            if (GraphicsClientItem *clientItem = qgraphicsitem_cast<GraphicsClientItem*>(*i)) {
                if (clientItem->isMacroItem()) {
                    scene->deleteClient(clientItem->getClientName());
                }
            }
        }
    }
}

void MainWindow::on_actionShow_module_controls_triggered()
{
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
        if (GraphicsClientItem *clientItem = qgraphicsitem_cast<GraphicsClientItem*>(*i)) {
            if (clientItem->isModuleItem()) {
                clientItem->setInnerItemVisible(true);
            }
        }
    }
    ui->actionShow_module_controls->setEnabled(false);
    ui->actionHide_module_controls->setEnabled(true);
}

void MainWindow::on_actionHide_module_controls_triggered()
{
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
        if (GraphicsClientItem *clientItem = qgraphicsitem_cast<GraphicsClientItem*>(*i)) {
            if (clientItem->isModuleItem()) {
                clientItem->setInnerItemVisible(false);
            }
        }
    }
    ui->actionShow_module_controls->setEnabled(true);
    ui->actionHide_module_controls->setEnabled(false);
}

void MainWindow::on_actionZoom_to_triggered()
{
    QRectF boundingSceneRect;
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QList<QGraphicsItem*>::iterator i = selectedItems.begin(); i != selectedItems.end(); i++) {
        boundingSceneRect |= (*i)->sceneBoundingRect();
    }
    ui->graphicsView->fitInView(boundingSceneRect, Qt::KeepAspectRatio);
}

void MainWindow::on_actionShow_all_triggered()
{
    QRectF boundingSceneRect;
    QList<QGraphicsItem*> items = scene->items();
    for (QList<QGraphicsItem*>::iterator i = items.begin(); i != items.end(); i++) {
        boundingSceneRect |= (*i)->sceneBoundingRect();
    }
    ui->graphicsView->fitInView(boundingSceneRect, Qt::KeepAspectRatio);
}
