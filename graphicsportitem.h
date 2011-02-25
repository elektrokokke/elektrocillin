#ifndef GRAPHICSPORTITEM_H
#define GRAPHICSPORTITEM_H

#include "jackclient.h"
#include "metajack/jack.h"
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QMenu>
#include <QMap>
#include <QObject>
#include <QBrush>
#include <QPen>
#include <QFont>

class GraphicsPortConnectionItem;

class GraphicsPortItem : public QObject, public QGraphicsPathItem, public PortConnectInterface
{
    Q_OBJECT
public:
    GraphicsPortItem(JackClient *client, const QString &fullPortName, QGraphicsItem *parent = 0);
    virtual ~GraphicsPortItem();

    void connectedTo(const QString &fullPortName);
    void disconnectedFrom(const QString &fullPortName);
    void registeredPort(const QString &fullPortname, const QString &type, int flags);
    void unregisteredPort(const QString &fullPortname, const QString &type, int flags);
protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
private slots:
    void onConnectAction();
    void onDisconnectAction();
private:
    JackClient *client;
    QString clientName;
    QString fullPortName, shortPortName, type;
    bool isInput;
    QGraphicsSimpleTextItem *portNameItem;
    QGraphicsEllipseItem *portConnectionPositionItem;
    int connections;
    QMenu contextMenu, *connectMenu, *disconnectMenu;
    QMap<QString, QAction*> mapPortNamesToActions;
    int gapSize;
    QBrush fill;
    QPen outline;
    QFont font;
};

class GraphicsPortConnectionItem : public QGraphicsPathItem
{
public:
    static GraphicsPortConnectionItem * getPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene);
    static void deletePortConnectionItem(const QString &port1, const QString &port2);
    void setPos(const QString &port, const QPointF &point);
    static void setPositions(const QString &port,const  QPointF &point);
private:
    GraphicsPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene);
    QString port1, port2;
    QPointF point1, point2;
    static QMap<QString, QMap<QString, GraphicsPortConnectionItem*> > items;
};

#endif // GRAPHICSPORTITEM_H
