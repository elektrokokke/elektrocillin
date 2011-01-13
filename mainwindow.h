#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SimpleMonophonicClient;

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

private:
    Ui::MainWindow *ui;

    SimpleMonophonicClient *simpleMonophonicClient;
};

#endif // MAINWINDOW_H
