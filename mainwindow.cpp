#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "iirmoogfilterclient.h"
#include "midisignalclient.h"
#include "linearwaveshapingclient.h"
#include "linearoscillatorclient.h"
#include "linearmorphoscillatorclient.h"
#include "cubicsplinewaveshapingclient.h"
#include "record2memoryclient.h"
#include "adsrclient.h"
#include "multiplyprocessor.h"
#include "whitenoisegenerator.h"
#include "envelopeclient.h"

#include <QDebug>
#include <QDialog>
#include <QBoxLayout>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings("settings.ini", QSettings::IniFormat),
    gridWidth(3),
    clientsRect(0, 0, 600, 420)
{   
    ui->setupUi(this);
    ui->graphicsView->setScene(new QGraphicsScene());

    addClient("system_in");
    addClient("system_out");

    Record2MemoryGraphicsItem *record2MemoryGraphicsItem = (Record2MemoryGraphicsItem*)addClient(record2MemoryClient = new Record2MemoryClient("Record"))->getInnerItem();
    // special treatment for the record client (its widget need resize when the scene scale changes):
    QObject::connect(ui->graphicsView, SIGNAL(animationFinished(QGraphicsView *)), record2MemoryGraphicsItem, SLOT(resizeForView(QGraphicsView *)));
    // be notified when something has been recorded (to update audio view):
    QObject::connect(record2MemoryClient, SIGNAL(recordingFinished()), this, SLOT(onRecordFinished()));
    recordClientGraphView = record2MemoryGraphicsItem->getGraphView();
    record2MemoryGraphicsItem->resizeForView(ui->graphicsView);

    for (QList<JackClientFactory*>::const_iterator i = JackClientFactory::getFactories().begin(); i != JackClientFactory::getFactories().end(); i++) {
        JackClientFactoryAction *action = new JackClientFactoryAction(*i, ui->menuCreate_client);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionCreateClient()));
        ui->menuCreate_client->addAction(action);
    }
//    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onActionAnimateToRect()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QGraphicsItem *graphicsItem = action->data().value<QGraphicsItem*>();
        ui->graphicsView->animateToVisibleSceneRect(graphicsItem->sceneBoundingRect());
    }
}

void MainWindow::onActionCreateClient()
{
    if (JackClientFactoryAction *action = qobject_cast<JackClientFactoryAction*>(sender())) {
        addClient(action->getFactory()->createClient(action->getFactory()->getName()));
    }
}

void MainWindow::on_actionStore_connections_triggered()
{
    qDebug() << nullClient.getConnections();
    settings.setValue("connections", nullClient.getConnections());
}

void MainWindow::on_actionRestore_connections_triggered()
{
    qDebug() << settings.value("connections").toStringList();
    nullClient.restoreConnections(settings.value("connections").toStringList());
}

void MainWindow::onRecordFinished()
{
    recordClientGraphView->setModel(record2MemoryClient->popAudioModel());
    recordClientGraphView->model()->setParent(recordClientGraphView);
//    double min = -1, max = 1;
//    for (int i = 0; i < recordClientGraphView->model()->rowCount(); i++) {
//        double value = recordClientGraphView->model()->data(recordClientGraphView->model()->index(i, 0)).toDouble();
//        if (value < min) {
//            min = value;
//        } else if (value > max) {
//            max = value;
//        }
//    }
//    for (int i = 0; i < recordClientGraphView->model()->rowCount(); i++) {
//        double value = recordClientGraphView->model()->data(recordClientGraphView->model()->index(i, 0)).toDouble();
//        recordClientGraphView->model()->setData(recordClientGraphView->model()->index(i, 0), (value - min) / (max - min) * 2.0 - 1.0, Qt::DisplayRole);
//    }
}

GraphicsClientItem * MainWindow::addClient(JackClient *client)
{
    int i = clients.size();
    clients.append(client);
    // activate the Jack client:
    clients[i]->activate();
    // create a visual representation and position it in the scene:
    int x = i % gridWidth;
    int y = i / gridWidth;
    QGraphicsItem *graphicsItem = clients[i]->createGraphicsItem(clientsRect);
    if (graphicsItem == 0) {
        // create "dummy" representation:
        graphicsItem = new QGraphicsSimpleTextItem(clients[i]->getClientName());
    }
    GraphicsClientItem *graphicsClientItem = new GraphicsClientItem(clients[i], clientsRect.translated(clientsRect.width() * x, clientsRect.height() * y));
    graphicsClientItem->setInnerItem(graphicsItem);
    ui->graphicsView->scene()->addItem(graphicsClientItem);
    // create an action to zoom to that client:
    QAction *action = ui->menuView->addAction(clients[i]->getClientName(), this, SLOT(onActionAnimateToRect()));
    action->setData(QVariant::fromValue<QGraphicsItem*>(graphicsItem));
    return graphicsClientItem;
}

GraphicsClientItem * MainWindow::addClient(const QString &clientName)
{
    int i = clients.size();
    clients.append(0);
    // create a visual representation and position it in the scene:
    int x = i % gridWidth;
    int y = i / gridWidth;
    QGraphicsItem *graphicsItem = new QGraphicsSimpleTextItem(clientName);
    GraphicsClientItem *graphicsClientItem = new GraphicsClientItem(&nullClient, clientName, clientsRect.translated(clientsRect.width() * x, clientsRect.height() * y));
    graphicsClientItem->setInnerItem(graphicsItem);
    ui->graphicsView->scene()->addItem(graphicsClientItem);
    // create an action to zoom to that client:
    QAction *action = ui->menuView->addAction(clientName, this, SLOT(onActionAnimateToRect()));
    action->setData(QVariant::fromValue<QGraphicsItem*>(graphicsItem));
    return graphicsClientItem;
}

void MainWindow::on_actionAll_modules_triggered()
{
    int x = gridWidth - 1;
    int y = (clients.size() - 1) / gridWidth;
    QRectF allClientsRect(clientsRect.topLeft(), clientsRect.translated(clientsRect.width() * x, clientsRect.height() * y).bottomRight());
    ui->graphicsView->animateToVisibleSceneRect(allClientsRect);
}
