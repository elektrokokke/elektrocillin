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

class Record2MemoryClient;
class MidiController2AudioClient;
class GraphicsNodeItem;

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
    void onMidiMessage(unsigned char, unsigned char, unsigned char);
    void onChangeCutoff(QPointF cutoffResonance);
    void onNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);

    void on_actionStore_connections_triggered();

    void on_actionRestore_connections_triggered();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    GraphicsNodeItem *cutoffResonanceNode;
    Record2MemoryClient *recordClient;
    MidiController2AudioClient *midiControllerClient;
    FrequencyResponseGraphicsItem *frequencyResponse;
    MidiSignalThread midiSignalThread;
    MonophonicSynthesizer synthesizer;
    MidiProcessorClient synthesizerClient;
    IirMoogFilter moogFilter;
    IirMoogFilter moogFilterCopy;
    IirMoogFilterClient moogFilterClient;
    Oscillator lfo;
    AudioProcessorClient lfoClient;
    JackNullClient nullClient;
};

#endif // MAINWINDOW_H
