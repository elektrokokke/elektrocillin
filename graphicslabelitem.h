#ifndef GRAPHICSLABELITEM_H
#define GRAPHICSLABELITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>

class GraphicsLabelItem : public QGraphicsRectItem
{
public:
    GraphicsLabelItem(QGraphicsItem *parent = 0);
    GraphicsLabelItem(const QString &text, QGraphicsItem *parent = 0);
    void setText(const QString &text);
    void setFont(const QFont &font);
private:
    QGraphicsSimpleTextItem *textItem;
    int padding;
};

#endif // GRAPHICSLABELITEM_H
