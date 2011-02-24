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
    void onActionAnimateToRect();
    void onActionCreateClient();
    void on_actionStore_connections_triggered();
    void on_actionRestore_connections_triggered();
    void onRecordFinished();
    void on_actionAll_modules_triggered();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    int gridWidth;
    QRectF clientsRect;
    QVector<JackClient*> clients;
    Record2MemoryClient *record2MemoryClient;
    GraphView *recordClientGraphView;
    JackNullClient nullClient;

    GraphicsClientItem * addClient(JackClient *client);
    GraphicsClientItem * addClient(const QString &clientName);
};

#endif // MAINWINDOW_H
