#include "graphicsclientitem.h"
#include "graphicsportitem.h"
#include "jackcontextgraphicsscene.h"
#include "metajack/recursivejackcontext.h"
#include <QFontMetrics>
#include <QPen>
#include <QLinearGradient>
#include <QGraphicsScene>
#include <QGraphicsView>

GraphicsClientItem::GraphicsClientItem(JackClient *client_, int type_, int portType_, QFont font_, QGraphicsItem *parent, JackContextGraphicsScene *scene) :
    QGraphicsPathItem(parent, scene),
    client(client_),
    clientName(client->getClientName()),
    type(type_),
    portType(portType_),
    font(font_),
    innerItem(0),
    context(RecursiveJackContext::getInstance()->getCurrentContext())
{
    init();
}
GraphicsClientItem::GraphicsClientItem(JackClient *client_, const QString &clientName_, int type_, int portType_, QFont font_, QGraphicsItem *parent, JackContextGraphicsScene *scene) :
    QGraphicsPathItem(parent, scene),
    client(client_),
    clientName(clientName_),
    type(type_),
    portType(portType_),
    font(font_),
    innerItem(0),
    context(RecursiveJackContext::getInstance()->getCurrentContext())
{
    init();
}

GraphicsClientItem::~GraphicsClientItem()
{
    RecursiveJackContext::getInstance()->setClientProperty(context, clientName, QVariant::fromValue<QPointF>(pos()));
    contextMenu->deleteLater();
}

const QString & GraphicsClientItem::getClientName() const
{
    return clientName;
}

const QRectF & GraphicsClientItem::getRect() const
{
    return rect;
}

void GraphicsClientItem::setInnerItem(QGraphicsItem *item)
{
    if (innerItem) {
        delete innerItem;
    }
    innerItem = item;
    showInnerItemCommand->setText("[+]");
    showInnerItemCommand->setVisible(innerItem);
    if (innerItem) {
        innerItem->setVisible(false);
        innerItem->setParentItem(this);
        innerItem->setPos(getRect().topRight());
    }
}

QGraphicsItem * GraphicsClientItem::getInnerItem() const
{
    return innerItem;
}

void GraphicsClientItem::showInnerItem(bool ensureVisible_)
{
    if (innerItem) {
        setFocus();
        if (!innerItem->isVisible()) {
            pathWithoutInnerItem = path();
        }
        // show the inner item if requested:
        innerItem->setVisible(ensureVisible_ || !innerItem->isVisible());
        if (innerItem->isVisible()) {
            setPath(pathWithoutInnerItem + RectanglePath(innerItem->boundingRect().translated(innerItem->pos())));
            showInnerItemCommand->setText("[-]");
        } else {
            setPath(pathWithoutInnerItem);
            showInnerItemCommand->setText("[+]");
        }
    }
}

void GraphicsClientItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    if (event->button() == Qt::RightButton) {
        event->accept();
        // show the context menu:
        contextMenu->exec(event->screenPos());
    } else {
        QGraphicsPathItem::mousePressEvent(event);
    }
}

void GraphicsClientItem::focusInEvent(QFocusEvent *)
{
    setZValue(1);
}

void GraphicsClientItem::focusOutEvent(QFocusEvent *)
{
    setZValue(0);
}

QVariant GraphicsClientItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged) {
        RecursiveJackContext::getInstance()->setClientProperty(context, clientName, QVariant::fromValue<QPointF>(pos()));
    }
    return QGraphicsItem::itemChange(change, value);
}

void GraphicsClientItem::onActionRemoveClient()
{
    // delete the client belonging to this item:
    if (JackContextGraphicsScene *jackContextGraphicsScene = qobject_cast<JackContextGraphicsScene*>(scene())) {
        jackContextGraphicsScene->deleteClient(client);
    }
}

void GraphicsClientItem::onActionEditMacro()
{
    // get the macro's wrapper client:
    JackContext *jackContext = RecursiveJackContext::getInstance()->getContextByClientName(clientName.toAscii().data());
    if (jackContext) {
        // get the current scene:
        JackContextGraphicsScene *oldScene = (JackContextGraphicsScene*)scene();
        // get the view of the scene:
        QList<QGraphicsView*> views = oldScene->views();
        // make the macro's wrapper client the new context:
        RecursiveJackContext::getInstance()->pushExistingContext(jackContext);
        // create a new scene in the new context and make it the current scene of all views:
        JackContextGraphicsScene *scene = new JackContextGraphicsScene();
        for (int i = 0; i < views.size(); i++) {
            views[i]->setScene(scene);
        }
        // delete the old scene:
        oldScene->deleteLater();
    }
}

void GraphicsClientItem::init()
{
    bool gradient = false;
    setCursor(Qt::ArrowCursor);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemSendsScenePositionChanges | QGraphicsItem::ItemIsFocusable);
    font.setStyleStrategy(QFont::PreferAntialias);
    QFont commandsFont = font;
    commandsFont.setBold(true);
    commandsFont.setStyleStrategy(QFont::PreferAntialias);
    QFontMetrics fontMetrics(font);
    int padding = fontMetrics.height() * 2;
    int portPadding = fontMetrics.height() / 2;
    bool isMacro = RecursiveJackContext::getInstance()->getContextByClientName(clientName.toAscii().data());

    contextMenu = new QMenu();
    if (isMacro) {
        contextMenu->addAction("Edit macro", this, SLOT(onActionEditMacro()));
        contextMenu->addSeparator();
    }
    contextMenu->addAction("Delete client", this, SLOT(onActionRemoveClient()));

    QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem(clientName, this);
    textItem->setFont(font);
    textItem->setPos(padding, padding);
    textItem->setZValue(1);
    showInnerItemCommand = new CommandTextItem("[+]", font, this);
    showInnerItemCommand->setPos(padding, padding + fontMetrics.lineSpacing());
    showInnerItemCommand->setVisible(innerItem);
    showInnerItemCommand->setZValue(1);
    QObject::connect(showInnerItemCommand, SIGNAL(triggered()), this, SLOT(showInnerItem()));

    QStringList inputPorts = client->getPorts(QString(clientName + ":.*").toAscii().data(), 0, JackPortIsInput);
    QList<GraphicsPortItem*> inputPortItems;
    int inputPortsWidth = -portPadding;
    int minimumInputPortWidth = 0;
    for (int i = 0; i < inputPorts.size(); i++) {
        inputPortItems.append(new GraphicsPortItem(client, inputPorts[i], 3, font, this, (JackContextGraphicsScene*)scene()));
        if (isMacro) {
            QPen pen = inputPortItems.back()->pen();
            pen.setColor(QColor("steelblue"));
            inputPortItems.back()->setPen(pen);
        }
        inputPortsWidth += inputPortItems[i]->getRect().width() + portPadding;
        if ((i == 0) || (inputPortItems[i]->getRect().width() < minimumInputPortWidth)) {
            minimumInputPortWidth = inputPortItems[i]->getRect().width();
        }
    }
    QStringList outputPorts = client->getPorts(QString(clientName + ":.*").toAscii().data(), 0, JackPortIsOutput);
    QList<GraphicsPortItem*> outputPortItems;
    int outputPortsWidth = -portPadding;
    int minimumOutputPortWidth = 0;
    for (int i = 0; i < outputPorts.size(); i++) {
        outputPortItems.append(new GraphicsPortItem(client, outputPorts[i], 3, font, this, (JackContextGraphicsScene*)scene()));
        if (isMacro) {
            QPen pen = outputPortItems.back()->pen();
            pen.setColor(QColor("steelblue"));
            outputPortItems.back()->setPen(pen);
        }
        outputPortsWidth += outputPortItems[i]->getRect().width() + portPadding;
        if ((i == 0) || (outputPortItems[i]->getRect().width() < minimumOutputPortWidth)) {
            minimumOutputPortWidth = outputPortItems[i]->getRect().width();
        }
    }

    rect = textItem->boundingRect().adjusted(0, 0, padding * 2, padding * 2 + fontMetrics.height());
    if (rect.width() < inputPortsWidth + (portPadding - minimumInputPortWidth) * 2) {
        rect.setWidth(inputPortsWidth + (portPadding - minimumInputPortWidth) * 2);
    }
    if (rect.width() < outputPortsWidth + (portPadding - minimumOutputPortWidth) * 2) {
        rect.setWidth(outputPortsWidth + (portPadding - minimumOutputPortWidth) * 2);
    }

    if (gradient) {
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1, QColor(0xfc, 0xf9, 0xc2));//QColor("royalblue").lighter());
        setBrush(QBrush(gradient));
    }

    QPainterPath bodyPath;
    if (type == 0) {
        bodyPath = EllipsePath(rect);
    } else if (type == 1) {
        bodyPath = SpeechBubblePath(rect, rect.height() / 4, rect.height() / 4, Qt::AbsoluteSize);
    } else if (type == 2){
        bodyPath = RoundedRectanglePath(rect, padding + fontMetrics.height(), padding + fontMetrics.height());
    } else if (type == 3) {
        bodyPath = RectanglePath(rect);
    }

    for (int i = 0, x = (inputPortsWidth > rect.width() ? (rect.width() - inputPortsWidth) / 2 : 0); i < inputPorts.size(); i++) {
        GraphicsPortItem *portItem = inputPortItems[i];
        portItem->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
        portItem->setPos(x, 0);
        QRectF portRect(portItem->getRect().translated(portItem->pos()));

        QPainterPath portPath;
        portPath.addRect(QRectF(portRect.topLeft(), 0.5 * (portRect.topRight() + portRect.bottomRight())));
        if (portType == 0) {
            portPath += EllipsePath(portRect);
        } else if (portType == 1) {
            portPath += SpeechBubblePath(portRect, 0.7, 0.7);
        } else if (portType == 2) {
            portPath += RoundedRectanglePath(portRect, portPadding + fontMetrics.height() / 2, portPadding + fontMetrics.height() / 2);
        } else if (portType == 3) {
            portPath += RectanglePath(portRect);
        }
        bodyPath -= portPath;

        x += portRect.width() + portPadding;
    }
    for (int i = 0, x = (outputPortsWidth > rect.width() ? (rect.width() - outputPortsWidth) / 2 : 0); i < outputPorts.size(); i++) {
        GraphicsPortItem *portItem = outputPortItems[i];
        portItem->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
        portItem->setPos(x, rect.height() - fontMetrics.height());
        QRectF portRect(portItem->getRect().translated(portItem->pos()));

        QPainterPath portPath;
        portPath.addRect(QRectF(0.5 * (portRect.topLeft() + portRect.bottomLeft()), portRect.bottomRight()));
        if (portType == 0) {
            portPath += EllipsePath(portRect);
        } else if (portType == 1) {
            portPath += SpeechBubblePath(portRect, 0.7, 0.7);
        } else if (portType == 2) {
            portPath += RoundedRectanglePath(portRect, portPadding + fontMetrics.height() / 2, portPadding + fontMetrics.height() / 2);
        } else if (portType == 3) {
            portPath += RectanglePath(portRect);
        }
        bodyPath -= portPath;

        x += portRect.width() + portPadding;
    }
    QPainterPath combinedPath = bodyPath;

    QGraphicsPathItem *bodyItem = new QGraphicsPathItem(bodyPath, this);
    if (isMacro) {
        bodyItem->setPen(QPen(QBrush(QColor("steelblue")), 3));
    } else {
        bodyItem->setPen(QPen(QBrush(Qt::black), 3));
    }
    bodyItem->setBrush(QBrush(Qt::white));
    setPath(combinedPath);
    setPen(QPen(Qt::NoPen));
    setBrush(QBrush(Qt::NoBrush));

    // try to get the position:
    QVariant clientProperty = RecursiveJackContext::getInstance()->getClientProperty(clientName);
    if (clientProperty.isValid()) {
        setPos(clientProperty.toPointF());
    }
}
