#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "jacknullclient.h"

namespace Ui {
    class MainWindow;
}

class GraphicsClientItem;
class Record2MemoryClient;
class GraphView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:
    void on_actionLinear_oscillator_triggered();
    void onActionAnimateToRect();

    void on_actionStore_connections_triggered();

    void on_actionRestore_connections_triggered();

    void onRecordFinished();

    void on_actionADSR_envelope_triggered();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    int gridWidth;
    QVector<JackClient*> clients;
    Record2MemoryClient *record2MemoryClient;
    GraphView *recordClientGraphView;
    JackNullClient nullClient;

    GraphicsClientItem * addClient(JackClient *client);
    GraphicsClientItem * addClient(const QString &clientName);
};

#endif // MAINWINDOW_H
