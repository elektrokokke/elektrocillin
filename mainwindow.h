#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "frequencyresponsegraphicsitem.h"
#include "iirmoogfilterclient.h"
#include "iirbutterworthfilter.h"
#include "notetriggeredclient.h"
#include "monophonicsynthesizer.h"
#include "midisignalclient.h"

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
    void onRecordingStarted();
    void onRecordingFinished();

    void onMidiMessage(unsigned char, unsigned char, unsigned char);

    void onChangeCutoff(QPointF cutoffResonance);
    void onKeyPressed(unsigned char channel, unsigned char velocity, unsigned char noteNumber);

private:
    Ui::MainWindow *ui;
    Record2MemoryClient *recordClient;
    MidiController2AudioClient *midiControllerClient;
    FrequencyResponseGraphicsItem *frequencyResponse;
    IIRMoogFilter filterMoog;
    IIRButterworthFilter filterButterworth1, filterButterworth2, filterParallel, filterSerial;
    MidiSignalThread midiSignalThread;
    MonophonicSynthesizer synthesizer;
    NoteTriggeredClient synthesizerClient;
    IIRMoogFilterClient moogFilterClient;
};

#endif // MAINWINDOW_H
