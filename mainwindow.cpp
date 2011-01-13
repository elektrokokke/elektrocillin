#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simplemonophonicclient.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    simpleMonophonicClient(0)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionSimple_monophonic_triggered()
{
    if (simpleMonophonicClient) {
        delete simpleMonophonicClient;
        simpleMonophonicClient = 0;
        ui->actionSimple_monophonic->setChecked(false);
    } else {
        simpleMonophonicClient = new SimpleMonophonicClient("simple monophonic client");
        simpleMonophonicClient->activate();
        ui->actionSimple_monophonic->setChecked(true);
    }
}

void MainWindow::on_actionADSR_envelope_test_triggered()
{
    AdsrEnvelope envelope(0.05, 0.1, 0.75, 1.0);
    envelope.setSampleRate(100.0);
    envelope.noteOn();
    for (int sample = 0; sample < 200; sample++) {
        double level = envelope.nextSample();
        qDebug() << "sample" << sample << "level" << level;
        if (sample % 20 == 0) {
            envelope.noteOn();
        }
    }
}
