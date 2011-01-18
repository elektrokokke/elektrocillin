#ifndef MIDISPLITTER_H
#define MIDISPLITTER_H

#include <QObject>
#include <QMap>

class ControlChange : public QObject {
    Q_OBJECT
public:
    explicit ControlChange(QObject *parent = 0);
signals:
    void receivedControlChange(unsigned char channel, unsigned char controller, unsigned char value);
public slots:
    void onControlChange(unsigned char channel, unsigned char controller, unsigned char value);
};

class MidiSplitter : public QObject
{
    Q_OBJECT
public:
    explicit MidiSplitter(QObject *parent = 0);

    ControlChange * getControlChangeSink(unsigned char controller);

public slots:
    void splitControlChangeByController(unsigned char channel, unsigned char controller, unsigned char value);

private:
    QMap<unsigned char, ControlChange*> mapControllerToControlChange;
};

#endif // MIDISPLITTER_H
