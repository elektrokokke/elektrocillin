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
class JackContextGraphicsScene;

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
    void on_actionEdit_macro_triggered();
    void on_actionCreate_macro_triggered();
    void on_actionShow_all_controls_triggered();
    void on_actionHide_all_controls_triggered();
    void onSceneSelectionChanged();
    void on_actionDelete_module_triggered();
    void on_actionDelete_macro_triggered();
    void on_actionShow_module_controls_triggered();
    void on_actionHide_module_controls_triggered();

    void on_actionZoom_to_triggered();

    void on_actionShow_all_triggered();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    JackContextGraphicsScene *scene;
    QAction *playAction, *stopAction, *rewindAction;
};

#endif // MAINWINDOW_H
