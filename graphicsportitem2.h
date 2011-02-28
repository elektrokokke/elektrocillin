#ifndef GRAPHICSPORTITEM2_H
#define GRAPHICSPORTITEM2_H

#include "jackclient.h"
#include <QGraphicsPathItem>

class GraphicsPortItem2 : public QGraphicsPathItem, public PortConnectInterface
{
public:
    GraphicsPortItem2(JackClient *client, const QString &fullPortName, int style, QGraphicsItem *parent = 0);
    const QRectF & getRect() const;

    void connectedTo(const QString &fullPortName);
    void disconnectedFrom(const QString &fullPortName);
    void registeredPort(const QString &fullPortname, const QString &type, int flags);
    void unregisteredPort(const QString &fullPortname, const QString &type, int flags);
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
private:
    JackClient *client;
    QString fullPortName, shortPortName, dataType;
    bool isInput;
    int style;
    QRectF portRect;
};

#endif // GRAPHICSPORTITEM2_H
