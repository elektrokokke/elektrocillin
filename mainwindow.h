#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Record2MemoryClient;

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

private:
    Ui::MainWindow *ui;
    Record2MemoryClient *recordClient;
};

#endif // MAINWINDOW_H
