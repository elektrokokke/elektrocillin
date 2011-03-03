#ifndef JACKCONTEXTGRAPHICSSCENE_H
#define JACKCONTEXTGRAPHICSSCENE_H

#include "graphicsclientitem.h"
#include "graphicsportconnectionitem.h"
#include "jacknullclient.h"
#include <QGraphicsScene>
#include <QFont>

class JackContextGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    JackContextGraphicsScene(int clientStyle, int portStyle, const QFont &font);
    virtual ~JackContextGraphicsScene();

    void setClientStyle(int clientStyle);
    void setPortStyle(int portStyle);

    GraphicsClientItem * addClient(JackClient *client);
    GraphicsClientItem * addClient(const QString &clientName);
    void deleteClient(JackClient *client);
    void saveSession(QDataStream &stream);
    bool loadSession(QDataStream &stream);

    GraphicsPortConnectionItem * getPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene);
    void deletePortConnectionItem(const QString &port1, const QString &port2);
    void setPositions(const QString &port, const  QPointF &point);
    void deletePortConnectionItems(const QString &fullPortName);

protected:
    void deleteAllClients();

private:
    int clientStyle, portStyle;
    QFont font;
    QRectF clientsRect;
    JackNullClient nullClient;
    QMap<QString, QPair<JackClient*, GraphicsClientItem*> > clientsMap;
    QMap<QString, QMap<QString, GraphicsPortConnectionItem*> > portConnectionItems;
};

#endif // JACKCONTEXTGRAPHICSSCENE_H
