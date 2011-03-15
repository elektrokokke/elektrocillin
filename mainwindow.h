#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

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
    void onActionCreateClient();
    void on_actionSave_session_triggered();
    void on_actionLoad_session_triggered();
    void on_actionReset_triggered();
    void on_actionParent_level_triggered();
    void on_actionCreate_macro_triggered();
    void onSceneChanged();

    void on_actionShow_all_controls_triggered();

    void on_actionHide_all_controls_triggered();

private:
    Ui::MainWindow *ui;
    QSettings settings;
};

#endif // MAINWINDOW_H
