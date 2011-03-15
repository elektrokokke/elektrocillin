#ifndef OSCILLATORCLIENT_H
#define OSCILLATORCLIENT_H

#include "eventprocessorclient.h"
#include "oscillator.h"
#include "graphicsmeteritem.h"

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

    double getGain() const;
    void postChangeGain(double gain);
    double getTune() const;
    void postChangeTune(double tune);
    double getPitchModulationIntensity() const;
    void postChangePitchModulationIntensity(double halfTones);

    QGraphicsItem * createGraphicsItem();
private:
    Oscillator *oscillatorProcess;
    double gain, tune, pitchModulationIntensity;
};

class OscillatorClientGraphicsItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    OscillatorClientGraphicsItem(OscillatorClient *client, QGraphicsItem *parent = 0);
private slots:
    void onGainChanged(double value);
    void onDetuneChanged(double value);
    void onPitchModulationIntensityChanged(double value);
private:
    OscillatorClient *client;
};

#endif // OSCILLATORCLIENT_H
