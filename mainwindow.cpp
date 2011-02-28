#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "graphicsclientitem2.h"
#include "iirmoogfilterclient.h"

#include <QDebug>
#include <QDialog>
#include <QBoxLayout>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QGLWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings("settings.ini", QSettings::IniFormat),
    gridWidth(3),
    clientStyle(3),
    portStyle(1),
    clientFont("Mighty Zeo 2.0", 12),
//    clientFont("Helvetica", 12),
    clientsRect(0, 0, 600, 420)
{   
    ui->setupUi(this);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
    ui->graphicsView->setScene(new QGraphicsScene());
    ui->graphicsView->scene()->setBackgroundBrush(QBrush(QColor("lightsteelblue")));
//    ui->graphicsView->setViewport(new QGLWidget());

    addClient("system_in");
    addClient("system_out");

//    record2MemoryClient = new Record2MemoryClient("Record");
//    record2MemoryClient->activate();
//    Record2MemoryGraphicsItem *record2MemoryGraphicsItem = (Record2MemoryGraphicsItem*)addClient(record2MemoryClient)->getInnerItem();
//    // special treatment for the record client (its widget need resize when the scene scale changes):
//    QObject::connect(ui->graphicsView, SIGNAL(animationFinished(QGraphicsView *)), record2MemoryGraphicsItem, SLOT(resizeForView(QGraphicsView *)));
//    // be notified when something has been recorded (to update audio view):
//    QObject::connect(record2MemoryClient, SIGNAL(recordingFinished()), this, SLOT(onRecordFinished()));
//    recordClientGraphView = record2MemoryGraphicsItem->getGraphView();
//    record2MemoryGraphicsItem->resizeForView(ui->graphicsView);

    QList<JackClientFactory*> factories = JackClientFactory::getFactories();
    for (QList<JackClientFactory*>::const_iterator i = factories.begin(); i != factories.end(); i++) {
        JackClientFactoryAction *action = new JackClientFactoryAction(*i, ui->menuCreate_client);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(onActionCreateClient()));
        ui->menuCreate_client->addAction(action);
    }

//    JackClient *client = new IirMoogFilterClient("Lowpass resonance filter");
//    client->activate();
//    int x = 0;
//    for (int type = 0; type < 4; type++) {
//        QGraphicsSimpleTextItem *caption;
//        if (type == 0) {
//            caption = new QGraphicsSimpleTextItem("Ellipse");
//        } else if (type == 1) {
//            caption = new QGraphicsSimpleTextItem("Speech bubble");
//        } else if (type == 2) {
//            caption = new QGraphicsSimpleTextItem("Rounded rectangle");
//        } else if (type == 3) {
//            caption = new QGraphicsSimpleTextItem("Rectangle");
//        }
//        caption->setPos(x, -50);
//        ui->graphicsView->scene()->addItem(caption);
//        GraphicsClientItem2 *clientItem;
//        int y = 0;
//        for (int portType = 0; portType < 4; portType++) {
//            clientItem = new GraphicsClientItem2(client, type, portType);
//            clientItem->setPos(clientItem->getRect().width() * 1.25 * type, clientItem->getRect().height() * 1.25 * portType);
//            ui->graphicsView->scene()->addItem(clientItem);
//            if (type == 0) {
//                if (portType == 0) {
//                    caption = new QGraphicsSimpleTextItem("Ellipse");
//                } else if (portType == 1) {
//                    caption = new QGraphicsSimpleTextItem("Speech bubble");
//                } else if (portType == 2) {
//                    caption = new QGraphicsSimpleTextItem("Rounded rectangle");
//                } else if (portType == 3) {
//                    caption = new QGraphicsSimpleTextItem("Rectangle");
//                }
//                caption->setPos(-200, y);
//                ui->graphicsView->scene()->addItem(caption);
//            }
//            y += clientItem->getRect().height() * 1.25;
//        }
//        x += clientItem->getRect().width() * 1.25;
//    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onActionShowClient()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        GraphicsClientItem2 *graphicsClientItem = (GraphicsClientItem2*)action->data().value<QGraphicsItem*>();
        graphicsClientItem->showInnerItem(true);
    }
}

void MainWindow::onActionCreateClient()
{
    if (JackClientFactoryAction *action = qobject_cast<JackClientFactoryAction*>(sender())) {
        addClient(action->getFactory()->createClient(action->getFactory()->getName()));
    }
}

//void MainWindow::onRecordFinished()
//{
//    recordClientGraphView->setModel(record2MemoryClient->popAudioModel());
//    recordClientGraphView->model()->setParent(recordClientGraphView);
//}

GraphicsClientItem2 * MainWindow::addClient(JackClient *client)
{
    int i = clients.size();
    clients.append(client);
    // activate the Jack client:
    clients[i]->activate();
    // create a visual representation and position it in the scene:
    int x = i % gridWidth;
    int y = i / gridWidth;
    GraphicsClientItem2 *graphicsClientItem = new GraphicsClientItem2(clients[i], clientStyle, portStyle, clientFont);
    clientGraphicsItems.append(graphicsClientItem);
    graphicsClientItem->setPos(clientsRect.width() * x, clientsRect.height() * y);
    QGraphicsItem *graphicsItem = clients[i]->createGraphicsItem(clientsRect);
    if (graphicsItem) {
        graphicsClientItem->setInnerItem(graphicsItem);
        // create an action to zoom to that client:
        QAction *action = ui->menuView->addAction(clients[i]->getClientName(), this, SLOT(onActionShowClient()));
        action->setData(QVariant::fromValue<QGraphicsItem*>(graphicsClientItem));
    }
    ui->graphicsView->scene()->addItem(graphicsClientItem);
    return graphicsClientItem;
}

GraphicsClientItem2 * MainWindow::addClient(const QString &clientName)
{
    int i = clients.size();
    clients.append(0);
    // create a visual representation and position it in the scene:
    int x = i % gridWidth;
    int y = i / gridWidth;
    GraphicsClientItem2 *graphicsClientItem = new GraphicsClientItem2(&nullClient, clientName, clientStyle, portStyle, clientFont);
    clientGraphicsItems.append(graphicsClientItem);
    graphicsClientItem->setPos(clientsRect.width() * x, clientsRect.height() * y);
    ui->graphicsView->scene()->addItem(graphicsClientItem);
    return graphicsClientItem;
}

void MainWindow::on_actionAll_modules_triggered()
{
    int x = gridWidth - 1;
    int y = (clients.size() - 1) / gridWidth;
    QRectF allClientsRect(clientsRect.topLeft(), clientsRect.translated(clientsRect.width() * x, clientsRect.height() * y).bottomRight());
    ui->graphicsView->setVisibleSceneRect(allClientsRect);
}

void MainWindow::saveSession(QDataStream &stream)
{
    for (int i = 0; i < clients.size(); i++) {
        if (clients[i] && clients[i]->getFactory()) {
            // save the client's factory name:
            stream << clients[i]->getFactory()->getName();
        } else {
            stream << QString();
        }
        // save the client's name:
        stream << clientGraphicsItems[i]->getClientName();
        // save the client's position:
        stream << clientGraphicsItems[i]->pos();
        if (clients[i] && clients[i]->getFactory()) {
            // save the client's state:
            clients[i]->saveState(stream);
        }
    }
    // save two empty string as end token:
    stream << QString() << QString();
    // save the connections:
    stream << nullClient.getConnections();
}

bool MainWindow::loadSession(QDataStream &stream)
{
    QMap<QString, int> mapClientNameToIndex;
    // first delete all current clients:
    for (int i = 0; i < clients.size(); ) {
        if (clients[i] && clients[i]->getFactory()) {
            delete clientGraphicsItems[i];
            delete clients[i];
            clientGraphicsItems.remove(i);
            clients.remove(i);
        } else {
            mapClientNameToIndex[clientGraphicsItems[i]->getClientName()] = i;
            i++;
        }
    }
    // load the client names and create them:
    QString factoryName, clientName;
    stream >> factoryName >> clientName;
    for (; !factoryName.isNull() || !clientName.isNull(); ) {
        // load the client's position:
        QPointF position;
        stream >> position;
        if (factoryName.isNull()) {
            clientGraphicsItems[mapClientNameToIndex[clientName]]->setPos(position);
        } else {
            // get the factory associated with that name:
            JackClientFactory *factory = JackClientFactory::getFactoryByName(factoryName);
            if (!factory) {
                return false;
            }
            JackClient *client = factory->createClient(clientName);
            // load the client's state:
            client->loadState(stream);
            // show the client:
            addClient(client)->setPos(position);
        }
        stream >> factoryName >> clientName;
    }
    // load the connections and restore them:
    QStringList connections;
    stream >> connections;
    nullClient.restoreConnections(connections);
    return true;
}

void MainWindow::on_actionSave_session_triggered()
{
    // ask for the session file name:
    QString fileName = QFileDialog::getSaveFileName(this, "Save session", QString(), "Elektrocillin session files (*.elektrocillin)");
    if (!fileName.isNull()) {
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        QDataStream stream(&file);
        saveSession(stream);
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
        loadSession(stream);
    }
}

void MainWindow::on_actionReset_triggered()
{
    ui->graphicsView->resetTransform();
}
