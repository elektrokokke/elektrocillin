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

    QList<JackClientFactory*> factories = JackClientFactory::getFactories();
    for (QList<JackClientFactory*>::const_iterator i = factories.begin(); i != factories.end(); i++) {
        JackClientFactory *factory = *i;
        JackClientFactoryAction *action = new JackClientFactoryAction(factory, ui->menuCreate_client);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionCreateClient()));
        ui->menuCreate_client->addAction(action);
    }

    JackContextGraphicsScene *scene = new JackContextGraphicsScene(1, 3, QFont("Helvetica", 12));
    scene->setBackgroundBrush(QBrush(QColor("lightsteelblue")));

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
        GraphicsClientItem *graphicsClientItem = ((JackContextGraphicsScene*)ui->graphicsView->scene())->addClient(action->getFactory()->createClient(action->getFactory()->getName()));
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
    // get the current scene:
    JackContextGraphicsScene *oldScene = (JackContextGraphicsScene*)ui->graphicsView->scene();
    // pop the current jack context:
    RecursiveJackContext::getInstance()->popContext();
    // create a new scene in the new context and make it the current scene:
    JackContextGraphicsScene *scene = new JackContextGraphicsScene(1, 3, QFont("Helvetica", 12));
    scene->setBackgroundBrush(QBrush(QColor("lightsteelblue")));
    ui->graphicsView->setScene(scene);
    // delete the old scene:
    delete oldScene;
}

void MainWindow::on_actionCreate_macro_triggered()
{
    // get the current scene:
    JackContextGraphicsScene *oldScene = (JackContextGraphicsScene*)ui->graphicsView->scene();
    // create a new wrapper client:
    RecursiveJackContext::getInstance()->pushNewContext("New macro");
    // create a new scene in the new context and make it the current scene:
    JackContextGraphicsScene *scene = new JackContextGraphicsScene(1, 3, QFont("Helvetica", 12));
    scene->setBackgroundBrush(QBrush(QColor("lightsteelblue")));
    ui->graphicsView->setScene(scene);
    // delete the old scene:
    delete oldScene;
}
