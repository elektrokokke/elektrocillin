#ifndef TESTMIDICLIENTWIDGET_H
#define TESTMIDICLIENTWIDGET_H

#include "midiclientwidget.h"

namespace Ui {
    class TestMidiClientWidget;
}

class TestMidiClientWidget : public MidiClientWidget
{
    Q_OBJECT

public:
    explicit TestMidiClientWidget(QWidget *parent = 0);
    ~TestMidiClientWidget();

private:
    Ui::TestMidiClientWidget *ui;
};

#endif // TESTMIDICLIENTWIDGET_H
