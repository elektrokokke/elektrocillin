#ifndef OSCILLATORCLIENT_H
#define OSCILLATORCLIENT_H

#include "eventprocessorclient.h"
#include "oscillator.h"
#include <QGraphicsRectItem>

class OscillatorClient : public EventProcessorClient
{
public:
    /**
      Creates a new oscillator client object with the given name.

      This object takes ownership of the given Oscillator object, i.e., it will be deleted at destruction time.
      */
    OscillatorClient(const QString &clientName, Oscillator *oscillator, size_t ringBufferSize = 1024);
    virtual ~OscillatorClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    /**
      To call this method is only safe when the client is not running,
      as it accesses the internal Oscillator object used by the Jack
      process thread in a non-threadsafe way.

      To change the oscillator gain while the client is running use
      postChangeGain() method.
      */
    virtual void loadState(QDataStream &stream);

    Oscillator * getOscillator();

    void postChangeGain(double gain);
    void postChangeTune(double tune);
    void postChangePitchModulationIntensity(double halfTones);

    QGraphicsItem * createGraphicsItem();
private:
    Oscillator oscillator, *oscillatorProcess;
};

class OscillatorClientGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    OscillatorClientGraphicsItem(OscillatorClient *client, QGraphicsItem *parent = 0);
protected:
    virtual void focusInEvent(QFocusEvent * event);
    virtual void focusOutEvent(QFocusEvent * event);
private slots:
    void onGainChanged(double value);
    void onDetuneChanged(double value);
    void onPitchModulationIntensityChanged(double value);
private:
    OscillatorClient *client;
};

#endif // OSCILLATORCLIENT_H
