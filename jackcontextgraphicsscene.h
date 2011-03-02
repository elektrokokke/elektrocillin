#ifndef JACKCONTEXTGRAPHICSSCENE_H
#define JACKCONTEXTGRAPHICSSCENE_H

#include "graphicsclientitem2.h"
#include "jacknullclient.h"
#include <QGraphicsScene>
#include <QFont>

class JackContextGraphicsScene : public QGraphicsScene
{
public:
    JackContextGraphicsScene(int clientStyle, int portStyle, const QFont &font);

    void setClientStyle(int clientStyle);
    void setPortStyle(int portStyle);

    GraphicsClientItem2 * addClient(JackClient *client);
    void saveSession(QDataStream &stream);
    bool loadSession(QDataStream &stream);

protected:
    GraphicsClientItem2 * addClient(const QString &clientName);

private:
    int clientStyle, portStyle;
    QFont font;
    QRectF clientsRect;
    JackNullClient nullClient;
    QVector<JackClient*> clients;
    QVector<GraphicsClientItem2*> clientGraphicsItems;
};

#endif // JACKCONTEXTGRAPHICSSCENE_H
