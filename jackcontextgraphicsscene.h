#ifndef JACKCONTEXTGRAPHICSSCENE_H
#define JACKCONTEXTGRAPHICSSCENE_H

#include "graphicsclientitem.h"
#include "graphicsportconnectionitem.h"
#include "graphicsclientitemsclient.h"
#include "jacknullclient.h"
#include <QGraphicsScene>
#include <QFont>

class JackContext;

class JackContextGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    JackContextGraphicsScene();
    virtual ~JackContextGraphicsScene();

//    GraphicsClientItem * addClient(JackClient *client);
//    GraphicsClientItem * addClient(const QString &clientName);
    void deleteClient(const QString &clientName);
    void saveSession(QDataStream &stream);
    void loadSession(QDataStream &stream);

    GraphicsPortConnectionItem * getPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene);
    void deletePortConnectionItem(const QString &port1, const QString &port2);
    void setPositions(const QString &port, const  QPointF &point);
    void deletePortConnectionItems(const QString &fullPortName);

public slots:
    void clear();
    void showAllInnerItems(bool visible = true);

protected:
//    void deleteClients();

private:
//    JackNullClient nullClient;
//    QMap<QString, QPair<JackClient*, GraphicsClientItem*> > clientsMap;
    QMap<QString, QMap<QString, GraphicsPortConnectionItem*> > portConnectionItems;
    GraphicsClientItemsClient graphicsClientItemsClient;
};

#endif // JACKCONTEXTGRAPHICSSCENE_H
