#include "testmidiclientwidget.h"
#include "ui_testmidiclientwidget.h"
#include "simplemonophonicclient.h"

TestMidiClientWidget::TestMidiClientWidget(QWidget *parent) :
    MidiClientWidget(new SimpleMonophonicClient("synthesizer_test"), "midi in", parent),
    ui(new Ui::TestMidiClientWidget)
{
    ui->setupUi(this);

    QObject::connect(ui->pushButtonActivate, SIGNAL(clicked()), this, SLOT(activate()));
    QObject::connect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(close()));
}

TestMidiClientWidget::~TestMidiClientWidget()
{
    delete ui;
}
