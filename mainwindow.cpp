#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simplemonophonicclient.h"
#include "midi2signalclient.h"
#include "midicontrollerslider.h"
#include "midicontrollercheckbox.h"
#include "midisplitter.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    simpleMonophonicClient(0),
    midiInputDebugClient(0)
{
    ui->setupUi(this);

    splitter = new MidiSplitter(this);
    merger = new ControlChange(this);
    unsigned char controllerOffset[] = { 33, 65, 73 };
    for (int i = 0; i < 8; i++) {
        sliders[i] = new MidiControllerSlider(this);
        sliders[i]->setController(1+i);
        ui->gridLayout->addWidget(sliders[i], 0, i);
        QObject::connect(splitter->getControlChangeSink(sliders[i]->getController()), SIGNAL(receivedControlChange(unsigned char,unsigned char,unsigned char)), sliders[i], SLOT(onControlChange(unsigned char,unsigned char,unsigned char)));
        QObject::connect(sliders[i], SIGNAL(controlChanged(unsigned char,unsigned char,unsigned char)), merger, SLOT(onControlChange(unsigned char,unsigned char,unsigned char)));
        for (int j = 0; j < 3; j++) {
            checkBoxes[j * 8 + i] = new MidiControllerCheckBox(this);
            checkBoxes[j * 8 + i]->setController(controllerOffset[j] + i);
            ui->gridLayout->addWidget(checkBoxes[j * 8 + i], j+1, i);
            QObject::connect(splitter->getControlChangeSink(checkBoxes[j * 8 + i]->getController()), SIGNAL(receivedControlChange(unsigned char,unsigned char,unsigned char)), checkBoxes[j * 8 + i], SLOT(onControlChange(unsigned char,unsigned char,unsigned char)));
            QObject::connect(checkBoxes[j * 8 + i], SIGNAL(controlChanged(unsigned char,unsigned char,unsigned char)), merger, SLOT(onControlChange(unsigned char,unsigned char,unsigned char)));
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionSimple_monophonic_triggered()
{
    if (!simpleMonophonicClient) {
        simpleMonophonicClient = new SimpleMonophonicClient("simple monophonic client");
    }
    if (simpleMonophonicClient->isActive()) {
        simpleMonophonicClient->close();
    } else {
        simpleMonophonicClient->activate();
    }
    ui->actionSimple_monophonic->setChecked(simpleMonophonicClient->isActive());
}

void MainWindow::on_actionADSR_envelope_test_triggered()
{
    PulseOscillator osc;
    osc.setSampleRate(10);
    osc.setFrequency(4);
    for (int sample = 0; sample < 100; sample++) {
        qDebug() << "sample" << sample << "value" << osc.nextSample();
    }
}

void MainWindow::onNoteOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
    qDebug() << "onNoteOff" << channel << note << velocity;
}

void MainWindow::onNoteOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
    qDebug() << "onNoteOn" << channel << note << velocity;
}

void MainWindow::onAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    qDebug() << "onAfterTouch" << channel << note << pressure;
}

void MainWindow::onControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    qDebug() << "onControlChange" << channel << controller << value;
}

void MainWindow::onProgramChange(unsigned char channel, unsigned char program)
{
    qDebug() << "onProgramChange" << channel << program;
}

void MainWindow::onChannelPressure(unsigned char channel, unsigned char pressure)
{
    qDebug() << "onChannelPressure" << channel << pressure;
}

void MainWindow::onPitchWheel(unsigned char channel, int pitchCentered)
{
    qDebug() << "onPitchWheel" << channel << pitchCentered;
}

void MainWindow::on_actionMidi_input_debug_triggered()
{
    if (!midiInputDebugClient) {
        midiInputDebugClient = new Midi2SignalClient("midi input debug client");
        QObject::connect(midiInputDebugClient, SIGNAL(receivedControlChange(unsigned char,unsigned char,unsigned char)), this, SLOT(onControlChange(unsigned char,unsigned char,unsigned char)));
        QObject::connect(midiInputDebugClient, SIGNAL(receivedPitchWheel(unsigned char,int)), this, SLOT(onPitchWheel(unsigned char,int)));
        // connect the object to our midi splitter and merger:
        QObject::connect(midiInputDebugClient, SIGNAL(receivedControlChange(unsigned char,unsigned char,unsigned char)), splitter, SLOT(splitControlChangeByController(unsigned char,unsigned char,unsigned char)));
        QObject::connect(merger, SIGNAL(receivedControlChange(unsigned char,unsigned char,unsigned char)), midiInputDebugClient, SLOT(sendControlChange(unsigned char,unsigned char,unsigned char)));
    }
    if (midiInputDebugClient->isActive()) {
        midiInputDebugClient->close();
    } else {
        midiInputDebugClient->activate();
    }
    ui->actionMidi_input_debug->setChecked(midiInputDebugClient->isActive());
}
