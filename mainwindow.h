#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "frequencyresponsegraphicsitem.h"
#include "iirmoogfilterclient.h"
#include "iirbutterworthfilter.h"
#include "midiprocessorclient.h"
#include "monophonicsynthesizer.h"
#include "midisignalclient.h"
#include "oscillator.h"
#include "jacknullclient.h"
#include "graphicsclientitem.h"
#include "linearwaveshapingclient.h"
#include "linearoscillatorclient.h"
#include "linearmorphoscillatorclient.h"
#include "cubicsplinewaveshapingclient.h"
#include "record2memoryclient.h"
#include "adsrclient.h"
#include "multiplyprocessor.h"
#include "whitenoisegenerator.h"

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
    QAction *actionAllClients;
    JackNullClient nullClient;
    QRectF allClientsRect;

    GraphicsClientItem * addClient(JackClient *client);
};

#endif // MAINWINDOW_H
