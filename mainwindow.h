#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "iirbutterworthfilter.h"
#include "butterworth2polefilter.h"
#include "frequencyresponsegraphicsitem.h"
#include "iirmoogfilter.h"

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

    void on_horizontalSliderCutoff_valueChanged(int value);

    void on_horizontalSliderResonance_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    Record2MemoryClient *recordClient;
    SimpleMonophonicClient *simpleMonophonicClient;
    MidiController2AudioClient *midiControllerClient;
    FrequencyResponseGraphicsItem *frequencyResponse;
    Butterworth2PoleFilter filter1;
    IIRButterworthFilter filter2;
    IIRMoogFilter filter3;
};

#endif // MAINWINDOW_H
