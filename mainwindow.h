#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "frequencyresponsegraphicsitem.h"
#include "iirmoogfilter.h"
#include "iirbutterworthfilter.h"

class Record2MemoryClient;
class SimpleMonophonicClient;
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

    void onChangeCutoff(qreal hertz);
    void onChangeResonance(qreal resonance);
    void onKeyPressed(unsigned char channel, unsigned char velocity, unsigned char noteNumber);

private:
    Ui::MainWindow *ui;
    Record2MemoryClient *recordClient;
    SimpleMonophonicClient *simpleMonophonicClient;
    MidiController2AudioClient *midiControllerClient;
    FrequencyResponseGraphicsItem *frequencyResponse;
    IIRMoogFilter filterMoog;
    IIRButterworthFilter filterButterworth1, filterButterworth2, filterParallel, filterSerial;
};

#endif // MAINWINDOW_H
