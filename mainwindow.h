#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
    Copyright 2011 Arne Jacobs

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    void on_actionShow_all_controls_triggered();
    void on_actionHide_all_controls_triggered();
    void onSceneSelectionChanged();
    void on_actionDelete_module_triggered();
    void on_actionDelete_macro_triggered();
    void on_actionShow_module_controls_triggered();
    void on_actionHide_module_controls_triggered();
    void on_actionZoom_to_triggered();
    void on_actionShow_all_triggered();
    void onContextLevelChanged(int level);

    void on_actionNew_module_triggered();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    JackContextGraphicsScene *scene;
    QAction *playAction, *stopAction, *rewindAction;
};

#endif // MAINWINDOW_H
