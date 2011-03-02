#ifndef JACKCONTEXTGRAPHICSSCENE_H
#define JACKCONTEXTGRAPHICSSCENE_H

#include "graphicsclientitem2.h"
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

    GraphicsClientItem2 * addClient(JackClient *client);
    void removeClient(JackClient *client);
    void saveSession(QDataStream &stream);
    bool loadSession(QDataStream &stream);

protected:
    GraphicsClientItem2 * addClient(const QString &clientName);
    void removeAllClients();

private:
    int clientStyle, portStyle;
    QFont font;
    QRectF clientsRect;
    JackNullClient nullClient;
    QMap<QString, QPair<JackClient*, GraphicsClientItem2*> > clientsMap;
};

#endif // JACKCONTEXTGRAPHICSSCENE_H
