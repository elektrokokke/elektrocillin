#include "graphicsclientitem2.h"
#include "graphicsportitem2.h"
#include <QFontMetrics>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>

GraphicsClientItem2::GraphicsClientItem2(JackClient *client_, int type_, int portType_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    client(client_),
    clientName(client->getClientName()),
    type(type_),
    portType(portType_)
{
    init();
}
GraphicsClientItem2::GraphicsClientItem2(JackClient *client_, const QString &clientName_, int type_, int portType_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    client(client_),
    clientName(clientName_),
    type(type_),
    portType(portType_)
{
    init();
}

const QString & GraphicsClientItem2::getClientName() const
{
    return clientName;
}

const QRectF & GraphicsClientItem2::getRect() const
{
    return rect;
}

void GraphicsClientItem2::init()
{
    bool gradient = true;
    setCursor(Qt::ArrowCursor);
    setPen(QPen(QBrush(Qt::black), 3));
    setBrush(QBrush(Qt::white));
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemSendsScenePositionChanges);
    QFont font("Mighty Zeo 2.0", 12);
    font.setStyleStrategy(QFont::PreferAntialias);
    QFont commandsFont = font;
    commandsFont.setBold(true);
    commandsFont.setStyleStrategy(QFont::PreferAntialias);
    QFontMetrics fontMetrics(font);
    int padding = fontMetrics.height() * 2;
    int portPadding = fontMetrics.height() / 2;

    QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem(clientName, this);
    textItem->setFont(font);
    textItem->setPos(padding, padding);
    (new CommandTextItem("[+]", this))->setPos(padding, padding + fontMetrics.lineSpacing());

    rect = textItem->boundingRect().adjusted(0, 0, padding * 2, padding * 2 + fontMetrics.height());

    if (gradient) {
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1, QColor("royalblue").lighter());
        setBrush(QBrush(gradient));
    }

    QPainterPath path;
    if (type == 0) {
        path = EllipsePath(rect);
    } else if (type == 1) {
        path = SpeechBubblePath(rect, rect.height() / 4, rect.height() / 4, Qt::AbsoluteSize);
    } else if (type == 2){
        path = RoundedRectanglePath(rect, padding + fontMetrics.height(), padding + fontMetrics.height());
    } else if (type == 3) {
        path = RectanglePath(rect);
    }

    QStringList inputPorts = client->getPorts(QString(clientName + ":.*").toAscii().data(), 0, JackPortIsInput);
    QList<GraphicsPortItem2*> inputPortItems;
    int inpurtPortsWidth = -portPadding;
    for (int i = 0; i < inputPorts.size(); i++) {
        inputPortItems.append(new GraphicsPortItem2(client, inputPorts[i], 3, this));
        inpurtPortsWidth += inputPortItems[i]->getRect().width() + portPadding;
    }
    for (int i = 0, x = (inpurtPortsWidth > rect.width() ? (rect.width() - inpurtPortsWidth) / 2 : 0); i < inputPorts.size(); i++) {
        GraphicsPortItem2 *portItem = inputPortItems[i];
        portItem->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
        portItem->setPos(x, 0);
        QRectF portRect(portItem->getRect().translated(portItem->pos()));

        QPainterPath portPath;
        portPath.addRect(QRectF(portRect.topLeft(), 0.5 * (portRect.topRight() + portRect.bottomRight())));
        if (portType == 0) {
            portPath = portPath.united(EllipsePath(portRect));
        } else if (portType == 1) {
            portPath = portPath.united(SpeechBubblePath(portRect, portRect.height() / 4, portRect.height() / 4, Qt::AbsoluteSize));
        } else if (portType == 2) {
            portPath = portPath.united(RoundedRectanglePath(portRect, portPadding + fontMetrics.height() / 2, portPadding + fontMetrics.height() / 2));
        } else if (portType == 3) {
            portPath = portPath.united(RectanglePath(portRect));
        }
        path = path.subtracted(portPath);

        x += portRect.width() + portPadding;
    }

    QStringList outputPorts = client->getPorts(QString(clientName + ":.*").toAscii().data(), 0, JackPortIsOutput);
    QList<GraphicsPortItem2*> outputPortItems;
    int outputPortsWidth = -portPadding;
    for (int i = 0; i < outputPorts.size(); i++) {
        outputPortItems.append(new GraphicsPortItem2(client, outputPorts[i], 3, this));
        outputPortsWidth += outputPortItems[i]->getRect().width() + portPadding;
    }
    for (int i = 0, x = (outputPortsWidth > rect.width() ? (rect.width() - outputPortsWidth) / 2 : 0); i < outputPorts.size(); i++) {
        GraphicsPortItem2 *portItem = outputPortItems[i];
        portItem->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
        portItem->setPos(x, rect.height() - fontMetrics.height());
        QRectF portRect(portItem->getRect().translated(portItem->pos()));

        QPainterPath portPath;
        portPath.addRect(QRectF(0.5 * (portRect.topLeft() + portRect.bottomLeft()), portRect.bottomRight()));
        if (portType == 0) {
            portPath = portPath.united(EllipsePath(portRect));
        } else if (portType == 1) {
            portPath = portPath.united(SpeechBubblePath(portRect, 0.7, 0.7));
        } else if (portType == 2) {
            portPath = portPath.united(RoundedRectanglePath(portRect, portPadding + fontMetrics.height() / 2, portPadding + fontMetrics.height() / 2));
        } else if (portType == 3) {
            portPath = portPath.united(RectanglePath(portRect));
        }
        path = path.subtracted(portPath);

        x += portRect.width() + portPadding;
    }

    setPath(path);
}
