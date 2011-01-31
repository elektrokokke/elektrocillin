#ifndef GRAPHICSKEYBOARDITEM_H
#define GRAPHICSKEYBOARDITEM_H

#include <QGraphicsRectItem>
#include <QVector>

class GraphicsKeyItem;

class GraphicsKeyboardItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    explicit GraphicsKeyboardItem(QGraphicsItem *parent = 0);

signals:
    void keyPressed(unsigned char noteNumber);
    void keyReleased(unsigned char noteNumber);

public slots:
    void pressKey(unsigned char noteNumber);
    void releaseKey(unsigned char noteNumber);

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );

private:
    QVector<GraphicsKeyItem*> keys;
    GraphicsKeyItem *activeKey;
    int activeKeyIndex;
};

#endif // GRAPHICSKEYBOARDITEM_H
