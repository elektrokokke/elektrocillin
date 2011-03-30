#ifndef GRAPHICSLABELITEM_H
#define GRAPHICSLABELITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>

class GraphicsLabelItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    GraphicsLabelItem(QGraphicsItem *parent = 0);
    GraphicsLabelItem(const QString &text, QGraphicsItem *parent = 0);
    void setFont(const QFont &font);
public slots:
    void setText(QString text);
private:
    QGraphicsSimpleTextItem *textItem;
    int padding;
};

#endif // GRAPHICSLABELITEM_H
