#ifndef JACKCONTEXTGRAPHICSSCENE_H
#define JACKCONTEXTGRAPHICSSCENE_H

#include "graphicsclientitem.h"
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

    void deleteClient(const QString &clientName);
    void saveSession(QDataStream &stream);
    void loadSession(QDataStream &stream);

public slots:
    void showAllInnerItems(bool visible = true);
private:
    GraphicsClientItemsClient graphicsClientItemsClient;
};

#endif // JACKCONTEXTGRAPHICSSCENE_H
