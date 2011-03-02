#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "jacknullclient.h"
#include "jackcontextgraphicsscene.h"

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
    void onActionCreateClient();
    void on_actionSave_session_triggered();
    void on_actionLoad_session_triggered();
    void on_actionReset_triggered();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    JackContextGraphicsScene *scene;
};

#endif // MAINWINDOW_H
