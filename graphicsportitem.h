#ifndef GRAPHICSPORTITEM2_H
#define GRAPHICSPORTITEM2_H

#include "jackclient.h"
#include <QGraphicsPathItem>
#include <QMenu>

class JackContextGraphicsScene;

class GraphicsPortItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    GraphicsPortItem(JackClient *client, const QString &fullPortName, int style, QFont font, QGraphicsItem *parent, JackContextGraphicsScene *scene);
    const QRectF & getRect() const;
    QPointF getConnectionScenePos() const;

public slots:
    void onPortConnected(QString sourcePortName, QString destPortName);
    void onPortDisconnected(QString sourcePortName, QString destPortName);
    void onPortRegistered(QString fullPortname, QString type, int flags);
    void onPortUnregistered(QString fullPortname, QString type, int flags);

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
