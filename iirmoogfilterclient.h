#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    IirMoogFilterClient(const QString &clientName, IirMoogFilter *processFilter, IirMoogFilter *guiFilter, size_t ringBufferSize = 1024);
    virtual ~IirMoogFilterClient();

    IirMoogFilter * getMoogFilter();
    virtual QGraphicsItem * createGraphicsItem();
    virtual JackClientFactory * getFactory();
protected:
    // reimplemented from ParameterClient:
    virtual bool init();
private:
    IirMoogFilter *processFilter, *guiFilter;
};

class IirMoogFilterGraphicsItem : public FrequencyResponseGraphicsItem
{
    Q_OBJECT
public:
    IirMoogFilterGraphicsItem(IirMoogFilterClient *client, const QRectF &rect, QGraphicsItem *parent = 0);
protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
private:
    IirMoogFilterClient *client;
    GraphicsNodeItem *cutoffResonanceNode;
private slots:
    void onClientChangedParameters();
    void onGuiChangedFilterParameters(const QPointF &cutoffResonance);
};

#endif // IIRMOOGFILTERCLIENT_H
