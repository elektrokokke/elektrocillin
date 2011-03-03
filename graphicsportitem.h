#ifndef GRAPHICSPORTITEM2_H
#define GRAPHICSPORTITEM2_H

#include "jackclient.h"
#include <QGraphicsPathItem>
#include <QMenu>

class GraphicsPortItem : public QObject, public QGraphicsPathItem, public PortConnectInterface
{
    Q_OBJECT
public:
    GraphicsPortItem(JackClient *client, const QString &fullPortName, int style, QFont font, QGraphicsItem *parent = 0);
    const QRectF & getRect() const;

    void connectedTo(const QString &fullPortName);
    void disconnectedFrom(const QString &fullPortName);
    void registeredPort(const QString &fullPortname, const QString &type, int flags);
    void unregisteredPort(const QString &fullPortname, const QString &type, int flags);

    QPointF getConnectionScenePos() const;
protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
private slots:
    void onConnectAction();
    void onDisconnectAction();
private:
    JackClient *client;
    QString fullPortName, shortPortName, dataType;
    bool isInput;
    int style;
    QFont font;
    QRectF portRect;
    QMenu contextMenu, *connectMenu, *disconnectMenu;
    QMap<QString, QAction*> mapPortNamesToActions;
    int connections;
};

#endif // GRAPHICSPORTITEM2_H
