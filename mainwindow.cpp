#include "mainwindow.h"
#include "ui_mainwindow.h"
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
#include "graphview.h"
#include <QDebug>
#include <QDialog>
#include <QBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings("settings.ini", QSettings::IniFormat),
    midiSignalClient("Virtual keyboard"),
    moogFilter(44100, 1),
    moogFilterCopy(44100, 1),
    moogFilterClient("Moog filter", &moogFilter),
    lfoClient1("LFO", &lfo1),
    lfoClient2("LFO 2", &lfo2),
    noiseClient("White noise", &noiseGenerator),
    adsrClient("ADSR envelope", 0.001, 0.2, 0.2, 0.3),
    multiplierClient("Multiplier", &multiplier),
    linearWaveShapingClient("Linear waveshaping"),
    fmClient("FM", &fm),
    linearMorphOscillatorClient("Oscillator", 2.0),
    cubicSplineWaveShapingClient("Cubic spline waveshaping"),
    recordClient("Record")
{   
    ui->setupUi(this);
    QGraphicsScene * scene = new QGraphicsScene();

    QRectF rect(0, 0, 600, 420);

    // moog filter client and gui test setup:
    frequencyResponse = new FrequencyResponseGraphicsItem(rect, 22050.0 / 512.0, 22050, -30, 30);
    IirMoogFilter::Parameters moogFilterParameters = moogFilter.getParameters();
    moogFilterCopy.setParameters(moogFilterParameters);
    frequencyResponse->addFrequencyResponse(&moogFilterCopy);
    cutoffResonanceNode = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, frequencyResponse);
    cutoffResonanceNode->setScale(GraphicsNodeItem::LOGARITHMIC, GraphicsNodeItem::LINEAR);
    cutoffResonanceNode->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    cutoffResonanceNode->setBrush(QBrush(qRgb(52, 101, 164)));
    cutoffResonanceNode->setZValue(10);
    cutoffResonanceNode->setBounds(QRectF(frequencyResponse->getFrequencyResponseRectangle().topLeft(), QPointF(frequencyResponse->getFrequencyResponseRectangle().right(), frequencyResponse->getZeroDecibelY())));
    cutoffResonanceNode->setBoundsScaled(QRectF(QPointF(frequencyResponse->getLowestHertz(), 1), QPointF(frequencyResponse->getHighestHertz(), 0)));
    //cutoffResonanceNode->setPos(frequencyResponse->getFrequencyResponseRectangle().left(), frequencyResponse->getZeroDecibelY());
    onChangedParameters(moogFilterParameters.frequency, moogFilterParameters.resonance);
    QObject::connect(cutoffResonanceNode, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onChangeCutoff(QPointF)));
    QObject::connect(moogFilterClient.getMoogFilterThread(), SIGNAL(changedParameters(double, double)), this, SLOT(onChangedParameters(double, double)));
    moogFilterClient.activate();
    // end moog filter client and gui test setup

    // virtual keyboard and midi signal client test setup:
    GraphicsKeyboardItem *keyboard = new GraphicsKeyboardItem(1);
    QObject::connect(keyboard, SIGNAL(keyPressed(unsigned char,unsigned char,unsigned char)), midiSignalClient.getMidiSignalThread(), SLOT(sendNoteOn(unsigned char,unsigned char,unsigned char)));
    QObject::connect(keyboard, SIGNAL(keyReleased(unsigned char,unsigned char,unsigned char)), midiSignalClient.getMidiSignalThread(), SLOT(sendNoteOff(unsigned char,unsigned char,unsigned char)));
    midiSignalClient.activate();
    // end virtual keyboard and midi signal client test setup

    // waveshaping clients test setup:
    LinearWaveShapingGraphicsItem *waveShapingItem = new LinearWaveShapingGraphicsItem(&linearWaveShapingClient, rect);
    linearWaveShapingClient.activate();
    CubicSplineWaveShapingGraphicsItem *cubicSplineWaveShapingItem = new CubicSplineWaveShapingGraphicsItem(&cubicSplineWaveShapingClient, rect);
    cubicSplineWaveShapingClient.activate();
    // end waveshaping client test setup

    // piecewise linear oscillator test setup:
    LinearMorphOscillatorGraphicsItem *linearMorphOscillatorGraphicsItem = new LinearMorphOscillatorGraphicsItem(rect, &linearMorphOscillatorClient);
    linearMorphOscillatorClient.activate();
    fmClient.activate();
    // end piecewise linear oscillator test setup

    // lfo test setup:
    lfo1.setFrequency(0.1);
    lfo2.setFrequency(0.1);
    lfoClient1.activate();
    lfoClient2.activate();
    noiseClient.activate();
    // end lfo test setup

    // ADSR envelope test setup:
    adsrClient.activate();
    multiplierClient.activate();
    // end ADSR envelope test setup

    // record client test setup:
    QGraphicsRectItem *recordClientRect = new QGraphicsRectItem(rect);
    recordClient.activate();
    recordClientGraphView = new GraphView(0);
    recordClientGraphView->resize(500, 400);
    QGraphicsProxyWidget *recordClientGraphicsItem = new QGraphicsProxyWidget(recordClientRect);
    recordClientGraphicsItem->setWidget(recordClientGraphView);
    recordClientGraphicsItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    QObject::connect(&recordClient, SIGNAL(recordingFinished()), this, SLOT(onRecordFinished()));
    // end record client test setup

    // client graphics item test setup:
    graphicsClientItemFilter = new GraphicsClientItem(&moogFilterClient, rect);
    graphicsClientItemFilter->setInnerItem(frequencyResponse);
    scene->addItem(graphicsClientItemFilter);
    allClientsRect = frequencyResponse->sceneBoundingRect();
    graphicsClientItemKeyboard = new GraphicsClientItem(&midiSignalClient, rect.translated(rect.width(), 0));
    graphicsClientItemKeyboard->setInnerItem(keyboard);
    scene->addItem(graphicsClientItemKeyboard);
    allClientsRect |= keyboard->sceneBoundingRect();
    graphicsClientItemWaveShaping = new GraphicsClientItem(&linearWaveShapingClient, rect.translated(0, rect.height()));
    graphicsClientItemWaveShaping->setInnerItem(waveShapingItem);
    scene->addItem(graphicsClientItemWaveShaping);
    allClientsRect |= waveShapingItem->sceneBoundingRect();
    graphicsClientItemCubicSplineWaveShaping = new GraphicsClientItem(&cubicSplineWaveShapingClient, rect.translated(rect.width(), rect.height()));
    graphicsClientItemCubicSplineWaveShaping->setInnerItem(cubicSplineWaveShapingItem);
    scene->addItem(graphicsClientItemCubicSplineWaveShaping);
    allClientsRect |= cubicSplineWaveShapingItem->sceneBoundingRect();
    graphicsClientItemLinearOscillator = new GraphicsClientItem(&linearMorphOscillatorClient, rect.translated(rect.width() * 2, 0));
    graphicsClientItemLinearOscillator->setInnerItem(linearMorphOscillatorGraphicsItem);
    scene->addItem(graphicsClientItemLinearOscillator);
    allClientsRect |= linearMorphOscillatorGraphicsItem->sceneBoundingRect();
    graphicsClientItemRecord = new GraphicsClientItem(&recordClient, rect.translated(rect.width() * 2, rect.height()));
    graphicsClientItemRecord->setInnerItem(recordClientRect);
    scene->addItem(graphicsClientItemRecord);
    allClientsRect |= recordClientRect->sceneBoundingRect();
    // end client graphics item test setup

    // port connection test setup:
    QStringList connectionList;
    connectionList.append("system_in:midi::Oscillator:Midi in");
    connectionList.append("system_in:midi::FM:Midi in");
    connectionList.append("system_in:midi::Record:Midi in");
    connectionList.append("system_in:midi::ADSR envelope:Midi in");
    connectionList.append("system_in:midi::Moog filter:Midi in");
    connectionList.append("Virtual keyboard:Midi out::Oscillator:Midi in");
    connectionList.append("Virtual keyboard:Midi out::FM:Midi in");
    connectionList.append("Virtual keyboard:Midi out::ADSR envelope:Midi in");
    connectionList.append("Virtual keyboard:Midi out::Moog filter:Midi in");
    connectionList.append("Virtual keyboard:Midi out::Record:Midi in");
//    connectionList.append("Oscillator:Audio out::Cubic spline waveshaping:Audio in");
//    connectionList.append("Cubic spline waveshaping:Audio out::Multiplier:Factor 1");
    connectionList.append("Oscillator:Audio out::Multiplier:Factor 1");
//    connectionList.append("FM:Audio out::Oscillator:Pitch modulation");
    connectionList.append("ADSR envelope:Envelope out::Multiplier:Factor 2");
//    connectionList.append("LFO 2:Audio out::Oscillator:Morph modulation");
    connectionList.append("Multiplier:Product out::Moog filter:Audio in");
    connectionList.append("Moog filter:Audio out::system_out:audio");
    connectionList.append("Moog filter:Audio out::Record:Audio in");
//    connectionList.append("Multiplier:Product out::system_out:audio");
//    connectionList.append("Multiplier:Product out::Record:Audio in");
    nullClient.restoreConnections(connectionList);
    // end port connection test setup

//    ui->graphicsView->setRenderHints(QPainter::Antialiasing);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setSceneRect(scene->sceneRect().adjusted(-1000, -1000, 1000, 1000));
    ui->graphicsView->centerOn(allClientsRect.center());

    onAnimationFinished();
    QObject::connect(ui->graphicsView, SIGNAL(animationFinished()), this, SLOT(onAnimationFinished()));

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

void MainWindow::onChangedParameters(double frequency, double resonance)
{
    IirMoogFilter::Parameters parameters = moogFilterCopy.getParameters();
    parameters.frequency = frequency;
    parameters.resonance = resonance;
    moogFilterCopy.setParameters(parameters);
    cutoffResonanceNode->setXScaled(parameters.frequency);
    cutoffResonanceNode->setYScaled(parameters.resonance);
    frequencyResponse->updateFrequencyResponse(0);
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

void MainWindow::on_actionMoog_filter_triggered()
{
    ui->graphicsView->animateToClientItem(graphicsClientItemFilter);
}

void MainWindow::on_actionVirtual_keyboard_triggered()
{
    ui->graphicsView->animateToClientItem(graphicsClientItemKeyboard);
}

void MainWindow::on_actionAll_triggered()
{
    ui->graphicsView->animateToVisibleSceneRect(allClientsRect);
}

void MainWindow::on_actionLinear_waveshaping_triggered()
{
    ui->graphicsView->animateToClientItem(graphicsClientItemWaveShaping);
}

void MainWindow::on_actionCubic_spline_waveshaping_triggered()
{
    ui->graphicsView->animateToClientItem(graphicsClientItemCubicSplineWaveShaping);
}

void MainWindow::on_actionOscillator_triggered()
{
    ui->graphicsView->animateToClientItem(graphicsClientItemLinearOscillator);
}

void MainWindow::onRecordFinished()
{
    recordClientGraphView->setModel(recordClient.popAudioModel());
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

void MainWindow::onAnimationFinished()
{
    QRect recordClientGraphViewRect = ui->graphicsView->mapFromScene(graphicsClientItemRecord->getInnerItem()->sceneBoundingRect()).boundingRect();
    recordClientGraphView->resize(recordClientGraphViewRect.width(), recordClientGraphViewRect.height());
}
