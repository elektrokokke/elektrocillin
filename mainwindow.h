#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "frequencyresponsegraphicsitem.h"
#include "iirmoogfilterclient.h"
#include "iirbutterworthfilter.h"
#include "midiprocessorclient.h"
#include "monophonicsynthesizer.h"
#include "midisignalclient.h"
#include "oscillator.h"
#include "jacknullclient.h"
#include "graphicsclientitem.h"
#include "linearwaveshapingclient.h"
#include "linearoscillatorclient.h"
#include "cubicsplinewaveshapingclient.h"
#include "record2memoryclient.h"
#include "adsrclient.h"
#include "multiplyprocessor.h"
#include "whitenoisegenerator.h"

class GraphicsNodeItem;
class GraphView;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:
    void on_actionOscillator_triggered();
    void onMidiMessage(unsigned char, unsigned char, unsigned char);
    void onChangeCutoff(QPointF cutoffResonance);
    void onChangedParameters(double frequency, double resonance);

    void on_actionStore_connections_triggered();

    void on_actionRestore_connections_triggered();

    void on_actionMoog_filter_triggered();

    void on_actionVirtual_keyboard_triggered();

    void on_actionAll_triggered();

    void on_actionLinear_waveshaping_triggered();

    void on_actionCubic_spline_waveshaping_triggered();

    void onRecordFinished();
    void onAnimationFinished();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    GraphicsNodeItem *cutoffResonanceNode;
    FrequencyResponseGraphicsItem *frequencyResponse;
    MidiSignalClient midiSignalClient;
    IirMoogFilter moogFilter;
    IirMoogFilter moogFilterCopy;
    IirMoogFilterClient moogFilterClient;
    Oscillator lfo1, lfo2;
    WhiteNoiseGenerator noiseGenerator;
    AudioProcessorClient lfoClient1, lfoClient2, noiseClient;
    AdsrClient adsrClient;
    MultiplyProcessor multiplier;
    AudioProcessorClient multiplierClient;
    LinearWaveShapingClient linearWaveShapingClient;
    LinearOscillatorClient linearOscillatorClient;
    CubicSplineWaveShapingClient cubicSplineWaveShapingClient;
    Record2MemoryClient recordClient;
    GraphView *recordClientGraphView;
    JackNullClient nullClient;
    QRectF allClientsRect;
    GraphicsClientItem *graphicsClientItemFilter, *graphicsClientItemKeyboard, *graphicsClientItemWaveShaping, *graphicsClientItemCubicSplineWaveShaping, *graphicsClientItemLinearOscillator, *graphicsClientItemRecord;
};

#endif // MAINWINDOW_H
