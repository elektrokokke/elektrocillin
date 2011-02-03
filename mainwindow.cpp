#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "record2memoryclient.h"
#include "graphicslineitem.h"
#include "graphicsloglineitem.h"
#include "graphicsnodeitem.h"
#include "midicontroller2audioclient.h"
#include "frequencyresponsegraphicsitem.h"
#include "iirbutterworthfilter.h"
#include "graphicsnodeitem.h"
#include "graphicskeyboarditem.h"
#include "eventprocessorclient.h"
#include <QDebug>
#include <QDialog>
#include <QBoxLayout>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    midiSignalThread("midi signal client"),
    synthesizerClient("synthesizer", &synthesizer),
    moogFilterClient("moog filter", &moogFilter),
    lfoClient("lfo", &lfo)
{
    ui->setupUi(this);

    lfo.setFrequency(0.5);

    QGraphicsScene * scene = new QGraphicsScene();
    frequencyResponse = new FrequencyResponseGraphicsItem(QRectF(0, 0, 800, 600), 22050.0 / 2048.0, 22050, -60, 20);
    IirMoogFilter::Parameters moogFilterParameters;
    moogFilterParameters.frequency = frequencyResponse->getLowestHertz();
    moogFilterParameters.frequencyOffsetFactor = 16;
    moogFilterParameters.frequencyPitchBendFactor = 1;
    moogFilterParameters.frequencyModulationFactor = 1;
    moogFilterParameters.frequencyModulationIntensity = 1;
    moogFilterParameters.resonance = 0;
    moogFilter.setParameters(moogFilterParameters);
    moogFilterCopy.setParameters(moogFilterParameters);
    frequencyResponse->addFrequencyResponse(&moogFilterCopy);

    cutoffResonanceNode = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, frequencyResponse);
    cutoffResonanceNode->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    cutoffResonanceNode->setBrush(QBrush(qRgb(52, 101, 164)));
    cutoffResonanceNode->setZValue(10);
    cutoffResonanceNode->setBounds(QRectF(frequencyResponse->getFrequencyResponseRectangle().topLeft(), QPointF(frequencyResponse->getFrequencyResponseRectangle().right(), frequencyResponse->getZeroDecibelY())));
    cutoffResonanceNode->setBoundsScaled(QRectF(QPointF(frequencyResponse->getLowestHertz(), 1), QPointF(frequencyResponse->getHighestHertz(), 0)));
    cutoffResonanceNode->setPos(frequencyResponse->getFrequencyResponseRectangle().left(), frequencyResponse->getZeroDecibelY());
    QObject::connect(cutoffResonanceNode, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onChangeCutoff(QPointF)));

    scene->addItem(frequencyResponse);

    midiSignalThread.getClient()->activate();
    synthesizerClient.activate();
    moogFilterClient.activate();
    lfoClient.activate();

    GraphicsKeyboardItem *keyboard = new GraphicsKeyboardItem(1);
    scene->addItem(keyboard);
    QObject::connect(keyboard, SIGNAL(keyPressed(unsigned char, unsigned char, unsigned char)), this, SLOT(onNoteOn(unsigned char, unsigned char, unsigned char)));
    QObject::connect(keyboard, SIGNAL(keyPressed(unsigned char,unsigned char,unsigned char)), &midiSignalThread, SLOT(sendNoteOn(unsigned char,unsigned char,unsigned char)));
    QObject::connect(keyboard, SIGNAL(keyReleased(unsigned char,unsigned char,unsigned char)), &midiSignalThread, SLOT(sendNoteOff(unsigned char,unsigned char,unsigned char)));
    keyboard->setScale(frequencyResponse->boundingRect().width() / keyboard->sceneBoundingRect().width());
    keyboard->setPos(0, -keyboard->sceneBoundingRect().height() - 10);

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

void MainWindow::onMidiMessage(unsigned char m1, unsigned char m2, unsigned char m3)
{
    qDebug() << "received midi message" << m1 << m2 << m3;
}

void MainWindow::onChangeCutoff(QPointF cutoffResonance)
{
    IirMoogFilter::Parameters parameters = moogFilterCopy.getParameters();
    parameters.frequency = cutoffResonance.x();
    parameters.resonance = cutoffResonance.y();
    moogFilterClient.postEvent(parameters);
    moogFilterCopy.setParameters(parameters);
    frequencyResponse->updateFrequencyResponse(0);
}

void MainWindow::onNoteOn(unsigned char, unsigned char noteNumber, unsigned char)
{
    IirMoogFilter::Parameters parameters = moogFilterCopy.getParameters();
    parameters.frequency = moogFilterCopy.computeFrequencyFromMidiNoteNumber(noteNumber);
    moogFilterCopy.setParameters(parameters);
    cutoffResonanceNode->setXScaled(parameters.frequency);
    frequencyResponse->updateFrequencyResponse(0);
}
