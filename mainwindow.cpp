#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simplemonophonicclient.h"
#include "record2memoryclient.h"
#include "midi2signalclient.h"
#include "graphicslineitem.h"
#include "graphicsloglineitem.h"
#include "graphicsnodeitem.h"
#include "midicontroller2audioclient.h"
#include "testmidiclientwidget.h"
#include "frequencyresponsegraphicsitem.h"
#include "iirbutterworthfilter.h"
#include <QDebug>
#include <QDialog>
#include <QBoxLayout>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    filter0(22050.0 / 4096.0, 44100),
    filter1(22050.0 / 8192.0, 0, 44100),
    filter2(22050.0 / 4096.0, 0.1, 44100),
    filter3(22050.0 / 2048.0, 0.2, 44100),
    filter4(22050.0 / 1024.0, 0.3, 44100),
    filter5(22050.0 / 512.0, 0.4, 44100),
    filter6(22050.0 / 256.0, 0.5, 44100),
    filter7(22050.0 / 128.0, 0.6, 44100)
{
    ui->setupUi(this);

    QGraphicsScene * scene = new QGraphicsScene();
    frequencyResponse = new FrequencyResponseGraphicsItem(QRectF(0, 0, 1300, 800), 22050.0 / 8192.0, 22050, -60, 20);
    frequencyResponse->addFrequencyResponse(&filter0);
    frequencyResponse->addFrequencyResponse(&filter1);
    frequencyResponse->addFrequencyResponse(&filter2);
    frequencyResponse->addFrequencyResponse(&filter3);
    frequencyResponse->addFrequencyResponse(&filter4);
    frequencyResponse->addFrequencyResponse(&filter5);
    frequencyResponse->addFrequencyResponse(&filter6);
    frequencyResponse->addFrequencyResponse(&filter7);
    scene->addItem(frequencyResponse);
    frequencyResponse->updateFrequencyResponses();
    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
    ui->graphicsView->setScene(scene);

//    QDialog *dialog = new QDialog(this);
//    dialog->setLayout(new QHBoxLayout(dialog));
//    dialog->layout()->addWidget(new TestMidiClientWidget(dialog));
//    dialog->show();

//    QObject::connect(ui->spinBoxZoom, SIGNAL(valueChanged(int)), ui->audioView, SLOT(setHorizontalScale(int)));

//    ui->horizontalSlider_2->setController(1);
//    Midi2SignalClient *midiClient = new Midi2SignalClient("signals/slots", this);
//    midiClient->activate();
//    midiClient->connectPorts("system_midi:capture_4", "signals/slots:midi in");
//    //QObject::connect(midiClient, SIGNAL(receivedControlChange(unsigned char,unsigned char,unsigned char)), this, SLOT(onMidiMessage(unsigned char,unsigned char,unsigned char)));
//    QObject::connect(midiClient, SIGNAL(receivedControlChange(unsigned char,unsigned char,unsigned char)), ui->horizontalSlider_2, SLOT(onControlChange(unsigned char,unsigned char,unsigned char)));
//    QObject::connect(ui->horizontalSlider_2, SIGNAL(controlChanged(unsigned char,unsigned char,unsigned char)), midiClient, SLOT(sendControlChange(unsigned char,unsigned char,unsigned char)));
//    QObject::connect(midiClient, SIGNAL(receivedPitchWheel(unsigned char,uint)), ui->horizontalSlider, SLOT(onPitchWheel(unsigned char,uint)));

//    recordClient = new Record2MemoryClient("record");
//    recordClient->activate();
//    recordClient->connectPorts("system_midi:capture_2", "record:midi in");
//    QObject::connect(recordClient, SIGNAL(recordingStarted()), this, SLOT(onRecordingStarted()));
//    QObject::connect(recordClient, SIGNAL(recordingFinished()), this, SLOT(onRecordingFinished()));

//    simpleMonophonicClient = new SimpleMonophonicClient("synthesizer");
//    simpleMonophonicClient->activate();
//    simpleMonophonicClient->connectPorts("system_midi:capture_4", "synthesizer:midi in");
//    simpleMonophonicClient->connectPorts("synthesizer:audio out", "system:playback_1");
//    simpleMonophonicClient->connectPorts("synthesizer:audio out", "system:playback_2");
//    simpleMonophonicClient->connectPorts("synthesizer:audio out", "record:audio in");
//    simpleMonophonicClient->connectPorts("system_midi:capture_4", "record:midi in");

//    midiControllerClient = new MidiController2AudioClient("controller", 0, 1);
//    midiControllerClient->activate();
//    midiControllerClient->connectPorts("system_midi:capture_2", "controller:midi in");
//    midiControllerClient->connectPorts("controller:audio out", "record:audio in");

//    // add a GraphicsNodeItem to the graphics scene:
//    QGraphicsScene * scene = new QGraphicsScene();

//    QRectF bounds(0.0, 0.0, 600.0, 200.0);
//    scene->addRect(bounds.adjusted(-10.0, -10.0, 10.0, 10.0));

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
//    simpleMonophonicClient->close();
//    midiControllerClient->close();
    delete ui;
}

void MainWindow::onRecordingStarted()
{
    qDebug() << "onRecordingStarted()";
}

void MainWindow::onRecordingFinished()
{
    qDebug() << "onRecordingFinished()";
//    // get the new audio model and show it:
//    JackAudioModel *audioModel = recordClient->popAudioModel();
//    if (audioModel) {
//        if (ui->audioView->model()) {
//            ui->audioView->model()->deleteLater();
//        }
//        audioModel->setParent(ui->audioView);
//        ui->audioView->setModel(audioModel);
//    }
}

void MainWindow::onMidiMessage(unsigned char m1, unsigned char m2, unsigned char m3)
{
    qDebug() << "received midi message" << m1 << m2 << m3;
}

void MainWindow::on_horizontalSliderCutoff_valueChanged(int value)
{
    // change cutoff frequency of the IIR Moog filter:
    double cutoff = frequencyResponse->getLowestHertz() * exp(log(frequencyResponse->getHighestHertz() / frequencyResponse->getLowestHertz()) / 100.0 * value);
    filter1.setCutoffFrequency(cutoff);
    frequencyResponse->updateFrequencyResponse(1);
}

void MainWindow::on_horizontalSliderResonance_valueChanged(int value)
{
    // change the resonance of the IIR Moog filter:
    double resonance = value * 0.01;
    filter1.setResonance(resonance);
    frequencyResponse->updateFrequencyResponse(1);
}
