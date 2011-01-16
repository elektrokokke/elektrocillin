#ifndef MIDICONTROLLERCHECKBOX_H
#define MIDICONTROLLERCHECKBOX_H

#include <QCheckBox>

class MidiControllerCheckBox : public QCheckBox
{
    Q_OBJECT
    Q_PROPERTY(unsigned char channel READ getChannel WRITE setChannel)
    Q_PROPERTY(unsigned char controller READ getController WRITE setController)
public:
    explicit MidiControllerCheckBox(QWidget *parent = 0);

    unsigned char getChannel() const;
    unsigned char getController() const;

signals:
    void controlChanged(unsigned char channel, unsigned char controller, unsigned char value);

public slots:
    void setChannel(unsigned char channel);
    void setController(unsigned char controller);
    void onControlChange(unsigned char channel, unsigned char controller, unsigned char value);

private slots:
    void onToggled(bool checked);

private:
    char channel, controller;
    bool changingValue;
};

#endif // MIDICONTROLLERCHECKBOX_H
