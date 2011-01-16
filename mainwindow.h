#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SimpleMonophonicClient;
class Midi2SignalClient;
class MidiControllerSlider;
class MidiControllerCheckBox;
class MidiSplitter;
class ControlChange;

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
    void on_actionSimple_monophonic_triggered();
    void on_actionADSR_envelope_test_triggered();

    void onNoteOff(unsigned char channel, unsigned char note, unsigned char velocity);
    void onNoteOn(unsigned char channel, unsigned char note, unsigned char velocity);
    void onAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure);
    void onControlChange(unsigned char channel, unsigned char controller, unsigned char value);
    void onProgramChange(unsigned char channel, unsigned char program);
    void onChannelPressure(unsigned char channel, unsigned char pressure);
    void onPitchWheel(unsigned char channel, int pitchCentered);

    void on_actionMidi_input_debug_triggered();

private:
    Ui::MainWindow *ui;

    SimpleMonophonicClient *simpleMonophonicClient;
    Midi2SignalClient *midiInputDebugClient;

    MidiSplitter *splitter;
    ControlChange *merger;
    MidiControllerSlider *sliders[8];
    MidiControllerCheckBox *checkBoxes[24];
};

#endif // MAINWINDOW_H
