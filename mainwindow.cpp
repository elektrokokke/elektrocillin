#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simplemonophonicclient.h"
#include "record2memoryclient.h"
#include "midi2signalclient.h"
#include "graphicslineitem.h"
#include "graphicsloglineitem.h"
#include "graphicsnodeitem.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QObject::connect(ui->spinBoxZoom, SIGNAL(valueChanged(int)), ui->audioView, SLOT(setHorizontalScale(int)));

    recordClient = new Record2MemoryClient("record");
    recordClient->activate();
    QObject::connect(recordClient, SIGNAL(recordingStarted()), this, SLOT(onRecordingStarted()));
    QObject::connect(recordClient, SIGNAL(recordingFinished()), this, SLOT(onRecordingFinished()));

    simpleMonophonicClient = new SimpleMonophonicClient("synthesizer");
    simpleMonophonicClient->activate();
    if (simpleMonophonicClient->isActive()) {
        simpleMonophonicClient->connectPorts("system_midi:capture_4", "synthesizer:midi in");
        simpleMonophonicClient->connectPorts("synthesizer:audio out", "system:playback_1");
        simpleMonophonicClient->connectPorts("synthesizer:audio out", "system:playback_2");
        simpleMonophonicClient->connectPorts("synthesizer:audio out", "record:audio in");
        simpleMonophonicClient->connectPorts("system_midi:capture_4", "record:midi in");
    } else {
        qDebug() << "client could not be acivated...";
    }

    Midi2SignalClient *midiClient = new Midi2SignalClient("signals/slots", this);
    midiClient->activate();

    QObject::connect(ui->horizontalSlider, SIGNAL(pitchWheel(unsigned char,uint)), midiClient, SLOT(sendPitchWheel(unsigned char,uint)));
    QObject::connect(midiClient, SIGNAL(receivedPitchWheel(unsigned char,uint)), ui->horizontalSlider, SLOT(onPitchWheel(unsigned char,uint)));

    // add a GraphicsNodeItem to the graphics scene:
    QGraphicsScene * scene = new QGraphicsScene();

    QRectF bounds(0.0, 0.0, 600.0, 200.0);
    scene->addRect(bounds.adjusted(-10.0, -10.0, 10.0, 10.0));

    // ADSR envelope GUI test:
    int nrOfNodes = 5;
    QList<GraphicsNodeItem*> nodes;
    for (int i = 0; i < nrOfNodes; i++) {
        GraphicsNodeItem * nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0);
        nodeItem->setPos(i * (bounds.width() / (nrOfNodes + 2)), 200.0);
        nodeItem->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
        nodeItem->setBrush(QBrush(qRgb(52, 101, 164)));
        nodeItem->setZValue(1);
        scene->addItem(nodeItem);
        nodes.append(nodeItem);

        nodeItem->setBounds(bounds);
    }
    for (int i = 1; i < nrOfNodes; i++) {
        GraphicsLogLineItem * line = new GraphicsLogLineItem(0.0, 0.0, 0.0, 0.0, (i+1) % 2);
        if ((i+1) % 2) {
            // toggle the logarithmic mode:
            QObject::connect(ui->checkBoxLogarithmic, SIGNAL(toggled(bool)), line, SLOT(setLogarithmic(bool)));
        }
        line->setPen(QPen(QBrush(qRgb(52, 101, 164)), 3));
        scene->addItem(line);
        nodes[i-1]->connectLine(line, GraphicsLineItem::P1);
        nodes[i]->connectLine(line, GraphicsLineItem::P2);
    }

    // restrict the five nodes:
    nodes[0]->setVisible(false);
    nodes[3]->setVisible(false);
    nodes[1]->setBounds(QRectF(nodes[0]->x(), bounds.top(), nodes[2]->x() - nodes[0]->x(), 0.0));
    nodes[2]->setBounds(QRectF(nodes[1]->x(), bounds.top(), nodes[3]->x() - nodes[1]->x(), bounds.height()));
    nodes[3]->setBounds(QRectF(nodes[3]->x(), bounds.top(), 0.0, bounds.height()));
    nodes[4]->setBounds(QRectF(nodes[3]->x(), bounds.bottom(), bounds.width() - nodes[3]->x(), 0.0));
    // third and fourth node keep the same vertical position:
    QObject::connect(nodes[3], SIGNAL(yChanged(qreal)), nodes[2], SLOT(setY(qreal)));
    QObject::connect(nodes[2], SIGNAL(yChanged(qreal)), nodes[3], SLOT(setY(qreal)));
    nodes[2]->setY(bounds.height() * 0.5);
    nodes[4]->setX(bounds.right());
    // connect the bounds of two neighboured nodes:
    QObject::connect(nodes[1], SIGNAL(xChanged(qreal)), nodes[2], SLOT(setBoundsLeft(qreal)));
    QObject::connect(nodes[2], SIGNAL(xChanged(qreal)), nodes[1], SLOT(setBoundsRight(qreal)));

    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
    ui->graphicsView->setScene(scene);
}

MainWindow::~MainWindow()
{
    simpleMonophonicClient->close();
    delete ui;
}

void MainWindow::onRecordingStarted()
{
    qDebug() << "onRecordingStarted()";
}

void MainWindow::onRecordingFinished()
{
    qDebug() << "onRecordingFinished()";
    // get the new audio model and show it:
    JackAudioModel *audioModel = recordClient->popAudioModel();
    if (audioModel) {
        if (ui->audioView->model()) {
            ui->audioView->model()->deleteLater();
        }
        audioModel->setParent(ui->audioView);
        ui->audioView->setModel(audioModel);
    }
}

void MainWindow::onMidiMessage(unsigned char, unsigned char, unsigned char)
{
    qDebug() << "received midi message";
}
