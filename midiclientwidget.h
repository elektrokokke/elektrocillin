#ifndef JACKMIDICLIENTWIDGET_H
#define JACKMIDICLIENTWIDGET_H

#include <QWidget>

class Midi2SignalClient;
class JackClient;

class MidiClientWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MidiClientWidget(JackClient *client, const QString &midiInputPortName, QWidget *parent = 0);
    virtual ~MidiClientWidget();

signals:

public slots:
    void activate();
    void close();

private:
    Midi2SignalClient *guiClient;
    JackClient *actualClient;
    QString midiInputPortName;
};

#endif // JACKMIDICLIENTWIDGET_H
