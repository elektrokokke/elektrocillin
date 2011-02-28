#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "jacknullclient.h"

namespace Ui {
    class MainWindow;
}

class GraphicsClientItem2;
class Record2MemoryClient;
class GraphView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:
    void onActionShowClient();
    void onActionCreateClient();
//    void onRecordFinished();
    void on_actionAll_modules_triggered();
    void on_actionSave_session_triggered();
    void on_actionLoad_session_triggered();
    void on_actionReset_triggered();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    int gridWidth, clientStyle, portStyle;
    QFont clientFont;
    QRectF clientsRect;
    QVector<JackClient*> clients;
    QVector<GraphicsClientItem2*> clientGraphicsItems;
    Record2MemoryClient *record2MemoryClient;
    GraphView *recordClientGraphView;
    JackNullClient nullClient;

    GraphicsClientItem2 * addClient(JackClient *client);
    GraphicsClientItem2 * addClient(const QString &clientName);

    void saveSession(QDataStream &stream);
    bool loadSession(QDataStream &stream);
};

#endif // MAINWINDOW_H
