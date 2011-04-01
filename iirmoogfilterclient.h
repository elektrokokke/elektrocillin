#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

#include "eventprocessorclient.h"
#include "eventprocessorclient.h"
#include "parameterclient.h"
#include "iirmoogfilter.h"
#include "jackringbuffer.h"
#include "frequencyresponsegraphicsitem.h"
#include "graphicsnodeitem.h"
#include <QObject>

class IirMoogFilterClient : public ParameterClient
{
    Q_OBJECT
public:
    /**
      Creates a new Moog filter client object with the given name. An associated IirMoogFilterThread will be
      automatically created and also deleted at destruction time.

      This object takes ownership of the given IirMoogFilter object, i.e., it will be deleted at destruction time.
      */
    IirMoogFilterClient(const QString &clientName, IirMoogFilter *filter, size_t ringBufferSize = 1024);
    virtual ~IirMoogFilterClient();

    IirMoogFilter * getMoogFilter();
    virtual QGraphicsItem * createGraphicsItem();
    virtual JackClientFactory * getFactory();
private slots:
    void onClientChangedParameter(int index, double value);
private:
    IirMoogFilter *iirMoogFilterProcess, *iirMoogFilter;
};

class IirMoogFilterGraphicsItem : public QObject, public FrequencyResponseGraphicsItem
{
    Q_OBJECT
public:
    IirMoogFilterGraphicsItem(IirMoogFilterClient *client, const QRectF &rect, QGraphicsItem *parent = 0);
private:
    IirMoogFilterClient *client;
    GraphicsNodeItem *cutoffResonanceNode;
private slots:
    void onClientChangedParameters();
    void onGuiChangedFilterParameters(const QPointF &cutoffResonance);
};

#endif // IIRMOOGFILTERCLIENT_H
