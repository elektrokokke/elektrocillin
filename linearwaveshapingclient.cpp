#include "linearwaveshapingclient.h"
#include "graphicsnodeitem.h"
#include "graphicslineitem.h"
#include <QPen>

LinearWaveShapingClient::LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient<LinearWaveShapingParameters>(clientName, QStringList("audio_in"), QStringList("shaped_out"), ringBufferSize),
    interpolator(QVector<double>(3), QVector<double>(3))
{
    interpolator.getX()[0] = 0;
    interpolator.getY()[0] = 0;
    interpolator.getX()[1] = 0.5;
    interpolator.getY()[1] = 0.5;
    interpolator.getX()[2] = 1;
    interpolator.getY()[2] = 1;
}

LinearWaveShapingClient::~LinearWaveShapingClient()
{
    close();
}

void LinearWaveShapingClient::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    if (inputs[0] >= 0) {
        outputs[0] = interpolator.evaluate(inputs[0]);
    } else {
        outputs[0] = -interpolator.evaluate(-inputs[0]);
    }
}

void LinearWaveShapingClient::processEvent(const LinearWaveShapingParameters &event, jack_nframes_t)
{
    // set the interpolator parameters accordingly:
    interpolator.getX()[1] = event.x;
    interpolator.getY()[1] = event.y;
}

LinearWaveShapingGraphicsItem::LinearWaveShapingGraphicsItem(const QRectF &rect, LinearWaveShapingClient *client_, QGraphicsItem *parent) :
    QGraphicsRectItem(rect, parent),
    client(client_)
{
    setPen(QPen(QBrush(Qt::black), 2));
    // create dotted vertical lines:
    for (int i = 0; i <= 10; i++) {
        qreal x = (double)i / 10.0 * (rect.right() - rect.left()) + rect.left();
        (new QGraphicsLineItem(x, rect.top(), x, rect.bottom(), this))->setPen(QPen(Qt::DotLine));
    }
    // create dotted horizontal lines:
    for (int i = 0; i <= 10; i++) {
        qreal y = (double)i / 10.0 * (rect.top() - rect.bottom()) + rect.bottom();
        (new QGraphicsLineItem(rect.left(), y, rect.right(), y, this))->setPen(QPen(Qt::DotLine));
    }
    GraphicsNodeItem *nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    nodeItem->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    nodeItem->setBrush(QBrush(qRgb(52, 101, 164)));
    nodeItem->setZValue(1);
    nodeItem->setBounds(rect);
    nodeItem->setBoundsScaled(QRectF(QPointF(0, 1), QPointF(1, 0)));
    nodeItem->setXScaled(0.5);
    nodeItem->setYScaled(0.5);
    nodeItem->setSendPositionChanges(true);
    GraphicsLineItem *lineItem1 = new GraphicsLineItem(rect.left(), rect.bottom(), rect.right(), rect.top(), this);
    GraphicsLineItem *lineItem2 = new GraphicsLineItem(rect.top(), rect.right(), rect.right(), rect.top(), this);
    lineItem1->setPen(QPen(QBrush(Qt::black), 2));
    lineItem2->setPen(QPen(QBrush(Qt::black), 2));
    nodeItem->connectLine(lineItem1, GraphicsLineItem::P2);
    nodeItem->connectLine(lineItem2, GraphicsLineItem::P1);
    QObject::connect(nodeItem, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onNodePositionChanged(QPointF)));
}

void LinearWaveShapingGraphicsItem::onNodePositionChanged(QPointF position)
{

    LinearWaveShapingParameters parameters;
    parameters.x = position.x();
    parameters.y = position.y();
    client->postEvent(parameters);
}
