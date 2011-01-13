#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simplemonophonicclient.h"

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
