#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simplemonophonicclient.h"
#include "record2memoryclient.h"
#include "midi2signalclient.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QObject::connect(ui->spinBoxZoom, SIGNAL(valueChanged(int)), ui->audioView, SLOT(setHorizontalScale(int)));

    recordClient = new Record2MemoryClient("record");
    recordClient->activate();
    QObject::connect(recordClient, SIGNAL(recordingStarted()), this, SLOT(onRecordingStarted()));
    QObject::connect(recordClient, SIGNAL(recordingFinished()), this, SLOT(onRecordingFinished()));

    simpleMonophonicClient = new SimpleMonophonicClient("synthesizer");
    simpleMonophonicClient->activate();
    if (simpleMonophonicClient->isActive()) {
        simpleMonophonicClient->connectPorts("system_midi:capture_4", "synthesizer:midi in");
        simpleMonophonicClient->connectPorts("synthesizer:audio out", "system:playback_1");
        simpleMonophonicClient->connectPorts("synthesizer:audio out", "system:playback_2");
        simpleMonophonicClient->connectPorts("synthesizer:audio out", "record:audio in");
        simpleMonophonicClient->connectPorts("system_midi:capture_4", "record:midi in");
    } else {
        qDebug() << "client could not be acivated...";
    }

    Midi2SignalClient *midiClient = new Midi2SignalClient("signals/slots", this);
    midiClient->activate();

    QObject::connect(ui->horizontalSlider, SIGNAL(pitchWheel(unsigned char,uint)), midiClient, SLOT(sendPitchWheel(unsigned char,uint)));
    QObject::connect(midiClient, SIGNAL(receivedPitchWheel(unsigned char,uint)), ui->horizontalSlider, SLOT(onPitchWheel(unsigned char,uint)));
}

MainWindow::~MainWindow()
{
    simpleMonophonicClient->close();
    delete ui;
}

void MainWindow::onRecordingStarted()
{
    qDebug() << "onRecordingStarted()";
}

void MainWindow::onRecordingFinished()
{
    qDebug() << "onRecordingFinished()";
    // get the new audio model and show it:
    JackAudioModel *audioModel = recordClient->popAudioModel();
    if (audioModel) {
        if (ui->audioView->model()) {
            ui->audioView->model()->deleteLater();
        }
        audioModel->setParent(ui->audioView);
        ui->audioView->setModel(audioModel);
    }
}

void MainWindow::onMidiMessage(unsigned char, unsigned char, unsigned char)
{
    qDebug() << "received midi message";
}
