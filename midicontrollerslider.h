#ifndef MIDICONTROLLERSLIDER_H
#define MIDICONTROLLERSLIDER_H

#include <QSlider>

class MidiControllerSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(unsigned char channel READ getChannel WRITE setChannel)
    Q_PROPERTY(unsigned char controller READ getController WRITE setController)
public:
    explicit MidiControllerSlider(QWidget *parent = 0);

    unsigned char getChannel() const;
    unsigned char getController() const;

signals:
    void controlChanged(unsigned char channel, unsigned char controller, unsigned char value);

public slots:
    void setChannel(unsigned char channel);
    void setController(unsigned char controller);
    void onControlChange(unsigned char channel, unsigned char controller, unsigned char value);

private slots:
    void onValueChanged(int value);

private:
    char channel, controller;
    bool changingValue;

};

#endif // MIDICONTROLLERSLIDER_H
