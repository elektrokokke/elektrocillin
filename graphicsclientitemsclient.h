#ifndef GRAPHICSCLIENTITEMSCLIENT_H
#define GRAPHICSCLIENTITEMSCLIENT_H

#include "jackclient.h"
#include "graphicsclientitem.h"
#include "graphicsportconnectionitem.h"
#include <QGraphicsScene>
#include <QMap>

class GraphicsClientItemsClient : public JackClient
{
    Q_OBJECT
public:
    GraphicsClientItemsClient(QGraphicsScene *scene);
    virtual ~GraphicsClientItemsClient();

    void saveState(QDataStream &stream);
    void loadState(QDataStream &stream);

    void setClientStyle(int clientStyle);
    void setPortStyle(int portStyle);

    void clear();
    void deleteClient(const QString &clientName);
    void deleteClients();

    QGraphicsScene * getScene();

    void showAllInnerItems(bool visible = true);

    GraphicsPortConnectionItem * getPortConnectionItem(const QString &port1, const QString &port2);
    void deletePortConnectionItem(QString port1, QString port2);
    void setPositions(const QString &port, const  QPointF &point);

    void setClientItemPosition(const QString &clientName, QPointF pos);

public slots:
    void onClientRegistered(const QString &clientName);
    void onClientUnregistered(const QString &clientName);
    void onPortRegistered(QString fullPortName, QString type, int flags);
private:
    QGraphicsScene *scene;
    QMap<QString, GraphicsClientItem*> clientItems;
    QMap<QString, QPointF> clientItemPositionMap;
    QMap<QString, bool> clientItemVisibleMap;
    QMap<QString, QMap<QString, GraphicsPortConnectionItem*> > portConnectionItems;
    int clientStyle, portStyle;
    QFont font;
};

#endif // GRAPHICSCLIENTITEMSCLIENT_H
