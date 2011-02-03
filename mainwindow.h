#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "frequencyresponsegraphicsitem.h"
#include "iirmoogfilterclient.h"
#include "iirbutterworthfilter.h"
#include "midiprocessorclient.h"
#include "monophonicsynthesizer.h"
#include "midisignalclient.h"
#include "oscillator.h"

class Record2MemoryClient;
class MidiController2AudioClient;

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

private:
    Ui::MainWindow *ui;
    Record2MemoryClient *recordClient;
    MidiController2AudioClient *midiControllerClient;
    FrequencyResponseGraphicsItem *frequencyResponse;
    MidiSignalThread midiSignalThread;
    MonophonicSynthesizer synthesizer;
    MidiProcessorClient synthesizerClient;
    IIRMoogFilter moogFilter;
    IIRMoogFilter moogFilterCopy;
    IIRMoogFilterClient moogFilterClient;
    Oscillator lfo;
    AudioProcessorClient lfoClient;
};

#endif // MAINWINDOW_H
