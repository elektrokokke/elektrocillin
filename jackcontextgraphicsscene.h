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
    virtual ~JackContextGraphicsScene();

    void saveSession(QDataStream &stream);
    void loadSession(QDataStream &stream);

    void changeToCurrentContext();
    void deleteClient(const QString &clientName);

    bool editMacro(GraphicsClientItem *macroItem);
signals:
    void contextLevelChanged(int level);
public slots:
    void showAllInnerItems(bool visible = true);
    void play();
    void stop();
    void rewind();
    void exitCurrentMacro();
    void editSelectedMacro();
    void createNewMacro();
    void createNewModule(QString factoryName);
private:
    GraphicsClientItemsClient *graphicsClientItemsClient;
};

#endif // JACKCONTEXTGRAPHICSSCENE_H
