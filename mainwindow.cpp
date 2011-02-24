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
    gridWidth(3)
{   
    ui->setupUi(this);
    ui->graphicsView->setScene(new QGraphicsScene());

    addClient(new IirMoogFilterClient("Moog filter"));
    addClient(new MidiSignalClient("Virtual keyboard"));
    addClient(new LinearWaveShapingClient("Linear waveshaping"));
    addClient(new CubicSplineWaveShapingClient("Cubic spline waveshaping"));
    addClient(new LinearMorphOscillatorClient("Oscillator", 2.0));
    addClient(new WhiteNoiseGeneratorClient("White noise"));
    addClient(new MultiplyClient("Multiplier"));
    addClient(new EnvelopeClient("Envelope"));
    Record2MemoryGraphicsItem *record2MemoryGraphicsItem = (Record2MemoryGraphicsItem*)addClient(record2MemoryClient = new Record2MemoryClient("Record"))->getInnerItem();

    // special treatment for the record client (its widget need resize when the scene scale changes):
    QObject::connect(ui->graphicsView, SIGNAL(animationFinished(QGraphicsView *)), record2MemoryGraphicsItem, SLOT(resizeForView(QGraphicsView *)));
    // be notified when something has been recorded (to update audio view):
    QObject::connect(record2MemoryClient, SIGNAL(recordingFinished()), this, SLOT(onRecordFinished()));
    recordClientGraphView = record2MemoryGraphicsItem->getGraphView();

//    ui->graphicsView->setRenderHints(QPainter::Antialiasing);

//    // ADSR envelope GUI test:
//    int nrOfNodes = 5;
//    QList<GraphicsNodeItem*> nodes;
//    for (int i = 0; i < nrOfNodes; i++) {
//        GraphicsNodeItem * nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0);
//        nodeItem->setPos(i * (bounds.width() / (nrOfNodes + 2)), 200.0);
//        nodeItem->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
//        nodeItem->setBrush(QBrush(qRgb(52, 101, 164)));
//        nodeItem->setZValue(1);
//        scene->addItem(nodeItem);
//        nodes.append(nodeItem);

//        nodeItem->setBounds(bounds);
//    }
//    for (int i = 1; i < nrOfNodes; i++) {
//        GraphicsLogLineItem * line = new GraphicsLogLineItem(0.0, 0.0, 0.0, 0.0, (i+1) % 2);
//        if ((i+1) % 2) {
//            // toggle the logarithmic mode:
//            QObject::connect(ui->checkBoxLogarithmic, SIGNAL(toggled(bool)), line, SLOT(setLogarithmic(bool)));
//        }
//        line->setPen(QPen(QBrush(qRgb(52, 101, 164)), 3));
//        scene->addItem(line);
//        nodes[i-1]->connectLine(line, GraphicsLineItem::P1);
//        nodes[i]->connectLine(line, GraphicsLineItem::P2);
//    }

//    // restrict the five nodes:
//    nodes[0]->setVisible(false);
//    nodes[3]->setVisible(false);
//    nodes[1]->setBounds(QRectF(nodes[0]->x(), bounds.top(), nodes[2]->x() - nodes[0]->x(), 0.0));
//    nodes[2]->setBounds(QRectF(nodes[1]->x(), bounds.top(), nodes[3]->x() - nodes[1]->x(), bounds.height()));
//    nodes[3]->setBounds(QRectF(nodes[3]->x(), bounds.top(), 0.0, bounds.height()));
//    nodes[4]->setBounds(QRectF(nodes[3]->x(), bounds.bottom(), bounds.width() - nodes[3]->x(), 0.0));
//    // third and fourth node keep the same vertical position:
//    QObject::connect(nodes[3], SIGNAL(yChanged(qreal)), nodes[2], SLOT(setY(qreal)));
//    QObject::connect(nodes[2], SIGNAL(yChanged(qreal)), nodes[3], SLOT(setY(qreal)));
//    nodes[2]->setY(bounds.height() * 0.5);
//    nodes[4]->setX(bounds.right());
//    // connect the bounds of two neighboured nodes:
//    QObject::connect(nodes[1], SIGNAL(xChanged(qreal)), nodes[2], SLOT(setBoundsLeft(qreal)));
//    QObject::connect(nodes[2], SIGNAL(xChanged(qreal)), nodes[1], SLOT(setBoundsRight(qreal)));

//    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
//    ui->graphicsView->setScene(scene);

//    Butterworth2PoleFilter filter(0.05);
//    PulseOscillator osc;
//    osc.setSampleRate(44100);
//    osc.setFrequency(441);
//    FloatTableModel *model = new FloatTableModel(this);
//    model->insertColumn(0);
//    model->insertRows(0, 2000);
//    for (int i = 0; i < model->rowCount(); i++) {
//        float value = osc.nextSample();
//        model->setData(model->index(i, 0), filter.filter(value), Qt::DisplayRole);
//    }
//    ui->audioView->setModel(model);
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

void MainWindow::on_actionADSR_envelope_triggered()
{
    addClient(new EnvelopeClient("Envelope"));
}

GraphicsClientItem * MainWindow::addClient(JackClient *client)
{
    QRectF rect(0, 0, 600, 420);
    int i = clients.size();
    clients.append(client);
    // activate the Jack client:
    clients[i]->activate();
    // create a visual representation and position it in the scene:
    int x = i % gridWidth;
    int y = i / gridWidth;
    QGraphicsItem *graphicsItem = clients[i]->createGraphicsItem(rect);
    if (graphicsItem == 0) {
        // create "dummy" representation:
        graphicsItem = new QGraphicsSimpleTextItem(clients[i]->getClientName());
    }
    GraphicsClientItem *graphicsClientItem = new GraphicsClientItem(clients[i], rect.translated(rect.width() * x, rect.height() * y));
    graphicsClientItem->setInnerItem(graphicsItem);
    ui->graphicsView->scene()->addItem(graphicsClientItem);
    // create an action to zoom to that client:
    QAction *action = ui->menuView->addAction(clients[i]->getClientName(), this, SLOT(onActionAnimateToRect()));
    action->setData(QVariant::fromValue<QGraphicsItem*>(graphicsItem));
    return graphicsClientItem;
}

void MainWindow::on_actionLinear_oscillator_triggered()
{
    addClient(new LinearOscillatorClient("Linear oscillator"));
}
