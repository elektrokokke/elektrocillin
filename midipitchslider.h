#ifndef MIDIPITCHSLIDER_H
#define MIDIPITCHSLIDER_H

#include <QSlider>

class MidiPitchSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(unsigned char channel READ getChannel WRITE setChannel)
public:
    explicit MidiPitchSlider(QWidget *parent = 0);

    unsigned char getChannel() const;

signals:
    void pitchWheel(unsigned char channel, unsigned int pitch);

public slots:
    void setChannel(unsigned char channel);
    void onPitchWheel(unsigned char channel, unsigned int pitch);

protected:
    virtual void mouseReleaseEvent ( QMouseEvent * ev );

private slots:
    void onValueChanged(int value);

private:
    char channel;
    bool changingValue;
};

#endif // MIDIPITCHSLIDER_H
