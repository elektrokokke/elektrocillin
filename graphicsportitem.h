#ifndef GRAPHICSPORTITEM2_H
#define GRAPHICSPORTITEM2_H

#include "jackclient.h"
#include <QGraphicsPathItem>
#include <QMenu>

class GraphicsClientItemsClient;

class GraphicsPortItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    GraphicsPortItem(GraphicsClientItemsClient *client, const QString &fullPortName, int style, QFont font, int padding, QGraphicsItem *parent);
    const QRectF & getRect() const;
    QPointF getConnectionScenePos() const;

public slots:
    void onPortRegistered(QString fullPortName, QString type, int flags);
    void onPortUnregistered(QString fullPortName, QString type, int flags);
    void onPortConnected(QString sourcePortName, QString destPortName);
    void onPortDisconnected(QString sourcePortName, QString destPortName);

protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
private slots:
    void onConnectAction();
    void onDisconnectAction();
private:
    GraphicsClientItemsClient *client;
    QString fullPortName, shortPortName, dataType;
    bool isInput;
    int style;
    QFont font;
    QRectF portRect;
    QMenu contextMenu, *connectMenu, *disconnectMenu;
    QMap<QString, QAction*> mapPortNamesToActions;
    bool showMenu;
};

#endif // GRAPHICSPORTITEM2_H
