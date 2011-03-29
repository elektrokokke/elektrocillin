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
    void messageChanged(QString message);
public slots:
    void showAllInnerItems(bool visible = true);
    void play();
    void stop();
    void rewind();
    void exitCurrentMacro();
    void editSelectedMacro();
    void createNewMacro();
    void createNewModule(QString factoryName);
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent);
private:
    GraphicsClientItemsClient *graphicsClientItemsClient;
    bool waitForMacroPosition, waitForModulePosition;
    QString factoryName;

    void createNewMacro(QPointF pos);
    void createNewModule(QString factoryName, QPointF pos);
};

#endif // JACKCONTEXTGRAPHICSSCENE_H
