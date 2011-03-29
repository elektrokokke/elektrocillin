#ifndef GRAPHICSCLIENTITEMSCLIENT_H
#define GRAPHICSCLIENTITEMSCLIENT_H

#include "jackclient.h"
#include "graphicsclientitem.h"
#include "graphicsportconnectionitem.h"
#include <QGraphicsScene>
#include <QMap>
#include <QSettings>

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

    void deleteClient(const QString &clientName);

    QGraphicsScene * getScene();

    void showAllInnerItems(bool visible = true);

    GraphicsPortConnectionItem * getPortConnectionItem(const QString &port1, const QString &port2);
    void deletePortConnectionItem(QString port1, QString port2);
    void deletePortConnectionItems(QString port);
    void setPositions(const QString &port, const  QPointF &point);

    void setClientItemPositionByName(const QString &clientName, QPointF pos);
public slots:
    void onClientRegistered(const QString &clientName);
    void onClientUnregistered(const QString &clientName);
    void onPortRegistered(QString fullPortName, QString type, int flags);
private:
    QGraphicsScene *scene;
    QMap<QString, GraphicsClientItem*> clientItems;
    QMap<QString, QPointF> clientItemPositionMap;
    QMap<QString, QMap<QString, GraphicsPortConnectionItem*> > portConnectionItems;
    int clientStyle, portStyle;
    QFont font;
    QString contextName;

    static QSettings settings;
};

#endif // GRAPHICSCLIENTITEMSCLIENT_H
