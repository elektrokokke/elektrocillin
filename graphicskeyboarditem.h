#ifndef GRAPHICSKEYBOARDITEM_H
#define GRAPHICSKEYBOARDITEM_H

#include <QGraphicsRectItem>
#include <QVector>

class GraphicsKeyItem;

class GraphicsKeyboardItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    explicit GraphicsKeyboardItem(unsigned char channel, QGraphicsItem *parent = 0);

    unsigned char getChannel() const;

signals:
    void keyPressed(unsigned char channel, unsigned char velocity, unsigned char noteNumber);
    void keyReleased(unsigned char channel, unsigned char velocity, unsigned char noteNumber);

public slots:
    void pressKey(unsigned char channel, unsigned char velocity, unsigned char noteNumber);
    void releaseKey(unsigned char channel, unsigned char velocity, unsigned char noteNumber);

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );

private:
    QVector<GraphicsKeyItem*> keys;
    unsigned char channel;
    GraphicsKeyItem *activeKey;
    int activeKeyIndex;
};

#endif // GRAPHICSKEYBOARDITEM_H
