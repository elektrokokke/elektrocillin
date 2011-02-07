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
#include "cubicsplineinterpolator.h"
#include "linearinterpolator.h"
#include "graphicsinterpolationitem.h"
#include "linearwaveshapingclient.h"
#include <QDebug>
#include <QDialog>
#include <QBoxLayout>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings("settings.ini", QSettings::IniFormat),
    midiSignalThread("virtual keyboard"),
    synthesizerClient("synthesizer", &synthesizer),
    moogFilter(44100, 1),
    moogFilterCopy(44100, 1),
    moogFilterClient("moog filter", &moogFilter),
    lfoClient1("lfo", &lfo1),
    lfoClient2("lfo_2", &lfo2),
    linearWaveShapingClient("linear waveshaping"),
    cubicSplineWaveShapingClient("spline waveshaping")
{   
    ui->setupUi(this);
    QGraphicsScene * scene = new QGraphicsScene();

//    // interpolation test:
//    QVector<double> controlPointsX, controlPointsY;
//    controlPointsX.append(0);
//    controlPointsY.append(0);
//    controlPointsX.append(0.4);
//    controlPointsY.append(1);
//    controlPointsX.append(0.6);
//    controlPointsY.append(1);
//    controlPointsX.append(1);
//    controlPointsY.append(0);
//    controlPointsX.append(1.4);
//    controlPointsY.append(-1);
//    controlPointsX.append(1.6);
//    controlPointsY.append(-1);
//    controlPointsX.append(2);
//    controlPointsY.append(0);
//    LinearInterpolator linearInterpolator(controlPointsX, controlPointsY);
//    CubicSplineInterpolator splineInterpolator(controlPointsX, controlPointsY);
//    GraphicsInterpolationItem *interpolationItem = new GraphicsInterpolationItem(&linearInterpolator, 0.01);
//    interpolationItem->setScale(100);
//    interpolationItem->setPos(0, -200);
//    scene->addItem(interpolationItem);
//    GraphicsInterpolationItem *splineItem = new GraphicsInterpolationItem(&splineInterpolator, 0.01);
//    splineItem->setScale(100);
//    splineItem->setPos(200, -200);
//    scene->addItem(splineItem);
//    // end interpolation test

    // moog filter client and gui test setup:
    frequencyResponse = new FrequencyResponseGraphicsItem(QRectF(0, 0, 600, 600), 11025.0 / 512.0, 11025, -60, 30);
    IirMoogFilter::Parameters moogFilterParameters = moogFilter.getParameters();
    moogFilterParameters.frequency = frequencyResponse->getLowestHertz();
    moogFilter.setParameters(moogFilterParameters);
    moogFilterCopy.setParameters(moogFilterParameters);
    frequencyResponse->addFrequencyResponse(&moogFilterCopy);
    cutoffResonanceNode = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, frequencyResponse);
    cutoffResonanceNode->setScale(GraphicsNodeItem::LOGARITHMIC, GraphicsNodeItem::LINEAR);
    cutoffResonanceNode->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    cutoffResonanceNode->setBrush(QBrush(qRgb(52, 101, 164)));
    cutoffResonanceNode->setZValue(10);
    cutoffResonanceNode->setBounds(QRectF(frequencyResponse->getFrequencyResponseRectangle().topLeft(), QPointF(frequencyResponse->getFrequencyResponseRectangle().right(), frequencyResponse->getZeroDecibelY())));
    cutoffResonanceNode->setBoundsScaled(QRectF(QPointF(frequencyResponse->getLowestHertz(), 1), QPointF(frequencyResponse->getHighestHertz(), 0)));
    cutoffResonanceNode->setPos(frequencyResponse->getFrequencyResponseRectangle().left(), frequencyResponse->getZeroDecibelY());
    QObject::connect(cutoffResonanceNode, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onChangeCutoff(QPointF)));
    QObject::connect(moogFilterClient.getMoogFilterThread(), SIGNAL(changedParameters(double)), this, SLOT(onChangedParameters(double)));
    moogFilterClient.activate();
    // end moog filter client and gui test setup

    // virtual keyboard and midi signal client test setup:
    GraphicsKeyboardItem *keyboard = new GraphicsKeyboardItem(1);
    QObject::connect(keyboard, SIGNAL(keyPressed(unsigned char,unsigned char,unsigned char)), &midiSignalThread, SLOT(sendNoteOn(unsigned char,unsigned char,unsigned char)));
    QObject::connect(keyboard, SIGNAL(keyReleased(unsigned char,unsigned char,unsigned char)), &midiSignalThread, SLOT(sendNoteOff(unsigned char,unsigned char,unsigned char)));
    midiSignalThread.getClient()->activate();
    // end virtual keyboard and midi signal client test setup

    // waveshaping clients test setup:
    LinearWaveShapingGraphicsItem *waveShapingItem = new LinearWaveShapingGraphicsItem(QRectF(0, 0, 600, 600), &linearWaveShapingClient);
    linearWaveShapingClient.activate();
    CubicSplineWaveShapingGraphicsItem *cubicSplineWaveShapingItem = new CubicSplineWaveShapingGraphicsItem(QRectF(0, 0, 600, 600), &cubicSplineWaveShapingClient);
    cubicSplineWaveShapingClient.activate();
    // end waveshaping client test setup

    // monophonic synthesizer and lfo test setup:
    synthesizerClient.activate();
    lfo1.setFrequency(0.1);
    lfo2.setFrequency(20);
    lfoClient1.activate();
    lfoClient2.activate();
    // end monophonic synthesizer and lfo test setup

    // client graphics item test setup:
    QRectF rect(0, 0, 100, 100);
    graphicsClientItemFilter = new GraphicsClientItem(moogFilterClient.getClientName(), rect);
    graphicsClientItemFilter->setInnerItem(frequencyResponse);
    scene->addItem(graphicsClientItemFilter);
    graphicsClientItemKeyboard = new GraphicsClientItem(midiSignalThread.getClient()->getClientName(), rect.translated(rect.width(), 0));
    graphicsClientItemKeyboard->setInnerItem(keyboard);
    scene->addItem(graphicsClientItemKeyboard);
    graphicsClientItemWaveShaping = new GraphicsClientItem("Linear waveshaping", rect.translated(0, rect.height()));
    graphicsClientItemWaveShaping->setInnerItem(waveShapingItem);
    scene->addItem(graphicsClientItemWaveShaping);
    graphicsClientItemCubicSplineWaveShaping = new GraphicsClientItem("Cubic spline waveshaping", rect.translated(rect.width(), rect.height()));
    graphicsClientItemCubicSplineWaveShaping->setInnerItem(cubicSplineWaveShapingItem);
    scene->addItem(graphicsClientItemCubicSplineWaveShaping);
    // end client graphics item test setup

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

void MainWindow::onChangedParameters(double frequency)
{
    IirMoogFilter::Parameters parameters = moogFilterCopy.getParameters();
    parameters.frequency = frequency;
    moogFilterCopy.setParameters(parameters);
    cutoffResonanceNode->setXScaled(parameters.frequency);
    frequencyResponse->updateFrequencyResponse(0);
}

void MainWindow::on_actionStore_connections_triggered()
{
    settings.setValue("connections", nullClient.getAllConnections());
}

void MainWindow::on_actionRestore_connections_triggered()
{
    nullClient.restoreConnections(settings.value("connections").toStringList());
}

void MainWindow::on_actionMoog_filter_triggered()
{
    ui->graphicsView->animateToVisibleSceneRect(graphicsClientItemFilter->sceneBoundingRect());
}

void MainWindow::on_actionVirtual_keyboard_triggered()
{
    ui->graphicsView->animateToVisibleSceneRect(graphicsClientItemKeyboard->sceneBoundingRect());
}

void MainWindow::on_actionAll_triggered()
{
    ui->graphicsView->animateToVisibleSceneRect(ui->graphicsView->sceneRect());
}

void MainWindow::on_actionLinear_waveshaping_triggered()
{
    ui->graphicsView->animateToVisibleSceneRect(graphicsClientItemWaveShaping->sceneBoundingRect());
}

void MainWindow::on_actionCubic_spline_waveshaping_triggered()
{
    ui->graphicsView->animateToVisibleSceneRect(graphicsClientItemCubicSplineWaveShaping->sceneBoundingRect());
}
