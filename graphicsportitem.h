#ifndef GRAPHICSPORTITEM_H
#define GRAPHICSPORTITEM_H

#include "jackclient.h"
#include "metajack/jack.h"
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QMenu>
#include <QMap>
#include <QObject>

class GraphicsPortItem : public QObject, public QGraphicsRectItem, public PortConnectInterface
{
    Q_OBJECT
public:
    GraphicsPortItem(JackClient *client, const QString &fullPortName, QGraphicsItem *parent = 0);

    void connectedTo(const QString &fullPortName);
    void disconnectedFrom(const QString &fullPortName);
protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
private slots:
    void onConnectAction();
    void onDisconnectAction();
private:
    JackClient *client;
    QString fullPortName, shortPortName, type;
    bool isInput;
    QGraphicsSimpleTextItem *portNameItem;
    QMenu contextMenu, *connectMenu, *disconnectMenu;
    QMap<QString, QAction*> mapPortNamesToActions;
};

#endif // GRAPHICSPORTITEM_H
