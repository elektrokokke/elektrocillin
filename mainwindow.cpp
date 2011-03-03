#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QGLWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings("settings.ini", QSettings::IniFormat)
{
    scene = new JackContextGraphicsScene(1, 3, QFont("Helvetica", 12));
//    scene = new JackContextGraphicsScene(1, 3, QFont("Mighty Zeo 2.0", 12));

    ui->setupUi(this);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
//    ui->graphicsView->setViewport(new QGLWidget());
    scene->setBackgroundBrush(QBrush(QColor("lightsteelblue")));
    ui->graphicsView->setScene(scene);

    QList<JackClientFactory*> factories = JackClientFactory::getFactories();
    for (QList<JackClientFactory*>::const_iterator i = factories.begin(); i != factories.end(); i++) {
        JackClientFactoryAction *action = new JackClientFactoryAction(*i, ui->menuCreate_client);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionCreateClient()));
        ui->menuCreate_client->addAction(action);
    }
}

MainWindow::~MainWindow()
{
    delete scene;
    delete ui;
}
void MainWindow::onActionCreateClient()
{
    if (JackClientFactoryAction *action = qobject_cast<JackClientFactoryAction*>(sender())) {
        GraphicsClientItem *graphicsClientItem = scene->addClient(action->getFactory()->createClient(action->getFactory()->getName()));
        // create an action to zoom to that client:
        QAction *action = ui->menuView->addAction(graphicsClientItem->getClientName(), this, SLOT(onActionShowClient()));
        action->setData(QVariant::fromValue<QGraphicsItem*>(graphicsClientItem));

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
        scene->saveSession(stream);
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
        scene->loadSession(stream);
    }
}

void MainWindow::on_actionReset_triggered()
{
    ui->graphicsView->resetTransform();
}
