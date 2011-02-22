#ifndef GRAPHICSPORTITEM_H
#define GRAPHICSPORTITEM_H

#include "jackclient.h"
#include "metajack/jack.h"
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QMenu>
#include <QMap>
#include <QObject>

class GraphicsPortItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    GraphicsPortItem(JackClient *client, const QString &fullPortName, QGraphicsItem *parent = 0);
protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
private slots:
    void onConnectAction();
    void onDisconnectAction();
private:
    JackClient *client;
    QString fullPortName, shortPortName;
    QGraphicsSimpleTextItem *portNameItem;
    QMenu contextMenu, *connectMenu, *disconnectMenu;
    QMap<QString, QAction*> mapPortNamesToActions;
    QMap<QAction*, QString> mapActionsToPortNames;

    void portConnected(const QString &fullPortName);
    void portDisconnected(const QString &fullPortName);
    void portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect);
    static void portConnectCallback(jack_port_id_t a, jack_port_id_t b, int connect, void* arg);
};

#endif // GRAPHICSPORTITEM_H
