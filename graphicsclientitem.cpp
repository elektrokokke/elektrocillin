#include "graphicsclientitem.h"
#include "graphicsclientitemsclient.h"
#include "jackcontextgraphicsscene.h"
#include "graphicsportitem.h"
#include "wheelzoominggraphicsview.h"
#include "metajack/recursivejackcontext.h"
#include <QFontMetrics>
#include <QPen>
#include <QLinearGradient>
#include <QGraphicsScene>
#include <QGraphicsView>

GraphicsClientItem::GraphicsClientItem(GraphicsClientItemsClient *client_, const QString &clientName_, int type_, int portType_, QFont font_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    client(client_),
    isJackClient(false),
    clientName(clientName_),
    type(type_),
    portType(portType_),
    padding(4),
    font(font_),
    innerItem(0),
    isMacro(RecursiveJackContext::getInstance()->getContextByClientName(clientName.toAscii().data()))
{
    initItem();
    initRest();
}

GraphicsClientItem::~GraphicsClientItem()
{
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
    } else {
        contextMenu->clear();
        showInnerItemAction = contextMenu->addAction("Show controls", this, SLOT(showInnerItem()));
        if (isMacro || (clientName == client->getClientName())) {
            contextMenu->addSeparator();
            contextMenu->addAction("Delete client", this, SLOT(onActionRemoveClient()));
        }
    }
    innerItem = item;
    showInnerItemCommand->setText("[+]");
    showInnerItemCommand->setVisible(innerItem);
    if (innerItem) {
        innerItem->setVisible(false);
        innerItem->setParentItem(this);
        innerItem->setPos(getRect().topRight() + QPointF(0, padding));
    }
    showInnerItemAction->setVisible(innerItem);
    zoomToInnerItemCommand->setVisible(innerItem);
}

QGraphicsItem * GraphicsClientItem::getInnerItem() const
{
    return innerItem;
}

void GraphicsClientItem::showInnerItem(bool ensureVisible_)
{
    if (innerItem) {
        setFocus();
        // show the inner item if requested:
        innerItem->setVisible(ensureVisible_ || !innerItem->isVisible());
        if (innerItem->isVisible()) {
            showInnerItemCommand->setText("[-]");
            showInnerItemAction->setText("Hide controls");
            updateBounds();
        } else {
            showInnerItemCommand->setText("[+]");
            showInnerItemAction->setText("Show controls");
            setPath(pathWithoutInnerItem);
        }
    }
}

void GraphicsClientItem::updateBounds()
{
    if (innerItem->isVisible()) {
        setPath(pathWithoutInnerItem + RectanglePath(innerItem->boundingRect().adjusted(-padding, -padding, padding, padding).translated(innerItem->pos())));
//        QPainterPath innerItemPath = innerItem->shape().translated(innerItem->pos());
//        setPath(pathWithoutInnerItem + innerItemPath + pathStroker.createStroke(innerItemPath));
    }

}

void GraphicsClientItem::zoomToInnerItem()
{
    if (innerItem) {
        showInnerItem(true);
        scene()->views().first()->fitInView(innerItem, Qt::KeepAspectRatio);
    }
}

void GraphicsClientItem::updatePorts()
{
    initItem();
}

void GraphicsClientItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsPathItem::mousePressEvent(event);
    if (!event->isAccepted() && (event->button() == Qt::RightButton) && (contextMenu->actions().size())) {
        event->accept();
    }
}

void GraphicsClientItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    if ((event->button() == Qt::RightButton)  && (contextMenu->actions().size())) {
        // show the context menu:
        contextMenu->exec(event->screenPos());
    } else {
        QGraphicsPathItem::mouseReleaseEvent(event);
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
        RecursiveJackContext::getInstance()->setClientProperty(clientName, QVariant::fromValue<QPointF>(pos()));
    }
    return QGraphicsItem::itemChange(change, value);
}

void GraphicsClientItem::onActionRemoveClient()
{
    // delete the client belonging to this item:
    client->deleteClient(clientName);
}

void GraphicsClientItem::onActionEditMacro()
{
    // get the macro's wrapper client:
    JackContext *jackContext = RecursiveJackContext::getInstance()->getContextByClientName(clientName.toAscii().data());
    if (jackContext) {
        // get the current scene:
        QGraphicsScene *oldScene = scene();
        // get the view of the scene:
        QList<QGraphicsView*> views = oldScene->views();
        // make the macro's wrapper client the new context:
        RecursiveJackContext::getInstance()->pushExistingContext(jackContext);
        // create a new scene in the new context and make it the current scene of all views:
        JackContextGraphicsScene *scene = new JackContextGraphicsScene();
        for (int i = 0; i < views.size(); i++) {
            if (WheelZoomingGraphicsView *graphicsView = qobject_cast<WheelZoomingGraphicsView*>(views[i])) {
                graphicsView->setScene(scene);
            } else {
                views[i]->setScene(scene);
            }
        }
        // delete the old scene:
        oldScene->deleteLater();
    }
}

void GraphicsClientItem::initItem()
{
    // delete all children (except the inner item):
    QList<QGraphicsItem*> children = childItems();
    for (int i = 0; i < children.size(); i++) {
        if (children[i] != innerItem) {
            delete children[i];
        }
    }

    pathStroker.setWidth(this->padding * 2);
    bool gradient = false;
    setCursor(Qt::ArrowCursor);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemSendsScenePositionChanges | QGraphicsItem::ItemIsFocusable);
    font.setStyleStrategy(QFont::PreferAntialias);
    QFont commandsFont = font;
    commandsFont.setBold(true);
    commandsFont.setStyleStrategy(QFont::PreferAntialias);
    QFontMetrics fontMetrics(font);
    int portPadding = fontMetrics.height() / 3;
    int padding = fontMetrics.height() + portPadding * 2;

    QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem(clientName, this);
    textItem->setFont(font);
    textItem->setPos(padding, padding);
    textItem->setZValue(1);
    showInnerItemCommand = new CommandTextItem("[+]", font, this);
    showInnerItemCommand->setPos(padding, padding + fontMetrics.lineSpacing());
    showInnerItemCommand->setVisible(innerItem);
    showInnerItemCommand->setZValue(1);
    QObject::connect(showInnerItemCommand, SIGNAL(triggered()), this, SLOT(showInnerItem()));
    zoomToInnerItemCommand = new CommandTextItem("[Z]", font, this);
    zoomToInnerItemCommand->setPos(padding + fontMetrics.width("[+]"), padding + fontMetrics.lineSpacing());
    zoomToInnerItemCommand->setVisible(innerItem);
    zoomToInnerItemCommand->setZValue(1);
    QObject::connect(zoomToInnerItemCommand, SIGNAL(triggered()), this, SLOT(zoomToInnerItem()));

    QStringList inputPorts = client->getPorts(QString(clientName + ":.*").toAscii().data(), 0, JackPortIsInput);
    QList<GraphicsPortItem*> inputPortItems;
    int inputPortsWidth = -portPadding;
    int minimumInputPortWidth = 0;
    for (int i = 0; i < inputPorts.size(); i++) {
        inputPortItems.append(new GraphicsPortItem(client, inputPorts[i], 3, font, portPadding, this));
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
        outputPortItems.append(new GraphicsPortItem(client, outputPorts[i], 3, font, portPadding, this));
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

    rect = (textItem->boundingRect().translated(textItem->pos()) | showInnerItemCommand->boundingRect().translated(showInnerItemCommand->pos())).adjusted(-padding, -padding, padding, padding);
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

//    QGraphicsPathItem *bodyItem = new QGraphicsPathItem(bodyPath, this);
    if (isMacro) {
        /*bodyItem->*/setPen(QPen(QBrush(QColor("steelblue")), 3));
    } else {
        /*bodyItem->*/setPen(QPen(QBrush(Qt::black), 3));
    }
    /*bodyItem->*/setBrush(QBrush(Qt::white));
    setPath(combinedPath);
    pathWithoutInnerItem = combinedPath;
//    setPen(QPen(Qt::NoPen));
//    setBrush(QBrush(Qt::NoBrush));
//    bodyItem->setPen(QPen(Qt::NoPen));
//    bodyItem->setBrush(QBrush(Qt::NoBrush));

    // try to get the position:
    QVariant clientProperty = RecursiveJackContext::getInstance()->getClientProperty(clientName);
    if (clientProperty.isValid()) {
        setPos(clientProperty.toPointF());
    }
}

void GraphicsClientItem::initRest()
{
    contextMenu = new QMenu();
    // if it corresponds to a JackClient object, create the GUI for it also:
    jack_client_t *client = meta_jack_client_by_name(clientName.toAscii().data());
    JackClient * jackClient;
    if (client && (jackClient = JackClientSerializer::getInstance()->getClient(client))) {
        isJackClient = true;
        QGraphicsItem *graphicsItem = jackClient->createGraphicsItem();
        if (graphicsItem) {
            setInnerItem(graphicsItem);
        }
    }
    if (isMacro) {
        contextMenu->addAction("Edit macro", this, SLOT(onActionEditMacro()));
    }
    if (isMacro || isJackClient) {
        contextMenu->addSeparator();
        contextMenu->addAction("Delete client", this, SLOT(onActionRemoveClient()));
    }
}
