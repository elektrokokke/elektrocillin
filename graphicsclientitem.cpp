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

QSettings GraphicsClientItem::settings("settings.ini", QSettings::IniFormat);

GraphicsClientItem::GraphicsClientItem(GraphicsClientItemsClient *client_, const QString &clientName_, int type_, int portType_, QFont font_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent, client_->getScene()),
    client(client_),
    isJackClient(false),
    clientName(clientName_),
    type(type_),
    portType(portType_),
    font(font_),
    controlsItem(0),
    isMacro(RecursiveJackContext::getInstance()->getContextByClientName(clientName.toAscii().data()))
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemSendsScenePositionChanges | QGraphicsItem::ItemIsFocusable);
    setCursor(Qt::ArrowCursor);
    font.setStyleStrategy(QFont::PreferAntialias);

    initItem();
    initRest();
}

GraphicsClientItem::~GraphicsClientItem()
{
    settings.setValue("position/" + contextName + "/" + clientName, pos().toPoint());
    settings.setValue("visible/" + contextName + "/" + clientName, controlsItem && controlsItem->isVisible());
}

const QString & GraphicsClientItem::getClientName() const
{
    return clientName;
}

const QRectF & GraphicsClientItem::getRect() const
{
    return rect;
}

QGraphicsItem * GraphicsClientItem::getControlsItem() const
{
    return controlsItem;
}

bool GraphicsClientItem::isControlsVisible() const
{
    return controlsItem && controlsItem->isVisible();
}

void GraphicsClientItem::setControlsVisible(bool visible)
{
    if (controlsItem) {
        // show the inner item if requested:
        controlsItem->setVisible(visible);
        showControlsCommand->setText(visible ? "[-]" : "[+]");
    }
}

bool GraphicsClientItem::isMacroItem() const
{
    return isMacro;
}

bool GraphicsClientItem::isModuleItem() const
{
    return isJackClient;
}

void GraphicsClientItem::toggleControls(bool ensureVisible_)
{
    if (controlsItem) {
        setFocus();
        // show the inner item if requested:
        setControlsVisible(ensureVisible_ || !controlsItem->isVisible());
    }
}

void GraphicsClientItem::zoomToControls()
{
    if (controlsItem) {
        toggleControls(true);
        scene()->views().first()->fitInView(controlsItem, Qt::KeepAspectRatio);
    }
}

void GraphicsClientItem::updatePorts()
{
    initItem();
}

void GraphicsClientItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (isMacroItem()) {
        if (((JackContextGraphicsScene*)scene())->editMacro(this)) {
            event->accept();
            return;
        }
    } else if (isModuleItem()) {
        toggleControls();
        event->accept();
        return;
    }
    QGraphicsPathItem::mouseDoubleClickEvent(event);
}

void GraphicsClientItem::focusInEvent(QFocusEvent *)
{
    setZValue(1);
    if (!isMacroItem() && !isModuleItem()) {
        scene()->clearSelection();
    }
}

void GraphicsClientItem::focusOutEvent(QFocusEvent *)
{
    setZValue(0);
}

void GraphicsClientItem::initItem()
{
    // delete all children (except the inner item):
    QList<QGraphicsItem*> children = childItems();
    for (int i = 0; i < children.size(); i++) {
        if (children[i] != controlsItem) {
            if (GraphicsPortItem *portItem = qgraphicsitem_cast<GraphicsPortItem*>(children[i])) {
                portItem->deleteLater();
            } else {
                delete children[i];
            }
        }
    }

    bool gradient = false;
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
    showControlsCommand = new CommandTextItem(controlsItem && controlsItem->isVisible() ? "[-]" : "[+]", font, this);
    showControlsCommand->setPos(padding, padding + fontMetrics.lineSpacing());
    showControlsCommand->setVisible(controlsItem);
    showControlsCommand->setZValue(1);
    QObject::connect(showControlsCommand, SIGNAL(triggered()), this, SLOT(toggleControls()));
    zoomToControlsCommand = new CommandTextItem("[Z]", font, this);
    zoomToControlsCommand->setPos(padding + fontMetrics.width("[+]"), padding + fontMetrics.lineSpacing());
    zoomToControlsCommand->setVisible(controlsItem);
    zoomToControlsCommand->setZValue(1);
    QObject::connect(zoomToControlsCommand, SIGNAL(triggered()), this, SLOT(zoomToControls()));

    QStringList inputPorts = client->getPorts(QString(clientName + ":.*").toAscii().data(), 0, JackPortIsInput);
    QList<GraphicsPortItem*> inputPortItems;
    int inputPortsWidth = -portPadding;
    int minimumInputPortWidth = 0;
    for (int i = 0; i < inputPorts.size(); i++) {
        inputPortItems.append(new GraphicsPortItem(client, inputPorts[i], 3, font, portPadding, this));
        if (isMacroItem()) {
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
        if (isMacroItem()) {
            QPen pen = outputPortItems.back()->pen();
            pen.setColor(QColor("steelblue"));
            outputPortItems.back()->setPen(pen);
        }
        outputPortsWidth += outputPortItems[i]->getRect().width() + portPadding;
        if ((i == 0) || (outputPortItems[i]->getRect().width() < minimumOutputPortWidth)) {
            minimumOutputPortWidth = outputPortItems[i]->getRect().width();
        }
    }

    rect = (textItem->boundingRect().translated(textItem->pos()) | showControlsCommand->boundingRect().translated(showControlsCommand->pos())).adjusted(-padding, -padding, padding, padding);
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

    if (isMacroItem()) {
        setPen(QPen(QBrush(QColor("steelblue")), 3));
    } else {
        setPen(QPen(QBrush(Qt::black), 3));
    }
    setBrush(QBrush(Qt::white));
    setPath(combinedPath);
}

void GraphicsClientItem::initRest()
{
    // if it corresponds to a JackClient object, create the GUI for it also:
    jack_client_t *client = meta_jack_client_by_name(clientName.toAscii().data());
    JackClient * jackClient;
    if (client && (jackClient = JackClientSerializer::getInstance()->getClient(client))) {
        isJackClient = true;
        controlsItem = jackClient->createGraphicsItem();
        showControlsCommand->setText("[+]");
        showControlsCommand->setVisible(controlsItem);
        if (controlsItem) {
            controlsItem->setVisible(false);
            controlsItem->setParentItem(this);
            controlsItem->setPos(getRect().topRight());
        }
        zoomToControlsCommand->setVisible(controlsItem);
    }
    setFlag(QGraphicsItem::ItemIsSelectable, isJackClient || isMacroItem());
    contextName = RecursiveJackContext::getInstance()->getCurrentContext()->get_name();
    setPos(settings.value("position/" + contextName + "/" + clientName).toPoint());
    setControlsVisible(settings.value("visible/" + contextName + "/" + clientName).toBool());
}
