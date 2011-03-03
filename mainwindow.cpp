#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "jackcontextgraphicsscene.h"
#include "metajack/recursivejackcontext.h"

#include <QDebug>
#include <QFileDialog>
#include <QGLWidget>

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
        ui->menuCreate_client->addAction(action);
    }

    JackContextGraphicsScene *scene = new JackContextGraphicsScene();

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
        ((JackContextGraphicsScene*)ui->graphicsView->scene())->addClient(action->getFactory()->createClient(action->getFactory()->getName()));
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
    // delete the current scene:
    delete ui->graphicsView->scene();
    // pop the current jack context:
    RecursiveJackContext::getInstance()->popContext();
    // create a new scene in the new context and make it the current scene:
    ui->graphicsView->setScene(new JackContextGraphicsScene());
}

void MainWindow::on_actionCreate_macro_triggered()
{
    // delete the current scene:
    delete ui->graphicsView->scene();
    // create a new wrapper client:
    RecursiveJackContext::getInstance()->pushNewContext("New macro");
    // create a new scene in the new context and make it the current scene:
    ui->graphicsView->setScene(new JackContextGraphicsScene());
}
