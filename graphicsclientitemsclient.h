#ifndef GRAPHICSCLIENTITEMSCLIENT_H
#define GRAPHICSCLIENTITEMSCLIENT_H

#include "jackclient.h"
#include "graphicsclientitem.h"
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

    void showAllInnerItems(bool visible = true);

public slots:
    void onClientRegistered(const QString &clientName);
    void onClientUnregistered(const QString &clientName);
private:
    QGraphicsScene *scene;
    QMap<QString, GraphicsClientItem*> clientItems;
    int clientStyle, portStyle;
    QFont font;
};

#endif // GRAPHICSCLIENTITEMSCLIENT_H
