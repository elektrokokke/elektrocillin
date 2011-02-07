#include "cubicsplinewaveshapingclient.h"
#include "graphicsnodeitem.h"
#include "graphicslineitem.h"
#include <QPen>

CubicSplineWaveShapingClient::CubicSplineWaveShapingClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient<CubicSplineWaveShapingParameters>(clientName, QStringList("audio_in"), QStringList("shaped_out"), ringBufferSize),
    interpolator(QVector<double>(), QVector<double>(), QVector<double>())
{
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(0);
    xx.append(0.25);
    yy.append(.25);
    xx.append(0.75);
    yy.append(0.75);
    xx.append(1);
    yy.append(1);
    interpolator = CubicSplineInterpolator(xx, yy);
    deactivateMidiInput();
}

CubicSplineWaveShapingClient::~CubicSplineWaveShapingClient()
{
    close();
}

const CubicSplineInterpolator & CubicSplineWaveShapingClient::getInterpolator() const
{
    return interpolator;
}

void CubicSplineWaveShapingClient::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    if (inputs[0] >= 0) {
        outputs[0] = std::max(std::min(interpolator.evaluate(inputs[0]), 1.0), 0.0);
    } else {
        outputs[0] = -std::max(std::min(interpolator.evaluate(-inputs[0]), 1.0), 0.0);
    }
}

void CubicSplineWaveShapingClient::processEvent(const CubicSplineWaveShapingParameters &event, jack_nframes_t)
{
    // set the interpolator parameters accordingly:
    QVector<double> xx, yy, y2;
    xx.append(event.x[0]);
    xx.append(event.x[1]);
    xx.append(event.x[2]);
    xx.append(event.x[3]);
    yy.append(event.y[0]);
    yy.append(event.y[1]);
    yy.append(event.y[2]);
    yy.append(event.y[3]);
    y2.append(event.y2[0]);
    y2.append(event.y2[1]);
    y2.append(event.y2[2]);
    y2.append(event.y2[3]);
    interpolator = CubicSplineInterpolator(xx, yy, y2);
}

CubicSplineWaveShapingGraphicsItem::CubicSplineWaveShapingGraphicsItem(const QRectF &rect, CubicSplineWaveShapingClient *client_, QGraphicsItem *parent) :
    QGraphicsRectItem(rect, parent),
    client(client_),
    interpolator(client->getInterpolator())
{
    setPen(QPen(QBrush(Qt::black), 2));
    setBrush(QBrush(Qt::white));
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
    {
        GraphicsNodeItem *nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
        nodeItem->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
        nodeItem->setBrush(QBrush(qRgb(52, 101, 164)));
        nodeItem->setZValue(1);
        nodeItem->setBounds(QRectF(rect.left(), rect.top(), rect.width(), rect.height()));
        nodeItem->setBoundsScaled(QRectF(QPointF(0, 1), QPointF(1, 0)));
        nodeItem->setXScaled(interpolator.getX()[1]);
        nodeItem->setYScaled(interpolator.getY()[1]);
        nodeItem->setSendPositionChanges(true);
        QObject::connect(nodeItem, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onNodePositionChanged1(QPointF)));
    }
    {
        GraphicsNodeItem *nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
        nodeItem->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
        nodeItem->setBrush(QBrush(qRgb(52, 101, 164)));
        nodeItem->setZValue(1);
        nodeItem->setBounds(QRectF(rect.left(), rect.top(), rect.width(), rect.height()));
        nodeItem->setBoundsScaled(QRectF(QPointF(0, 1), QPointF(1, 0)));
        nodeItem->setXScaled(interpolator.getX()[2]);
        nodeItem->setYScaled(interpolator.getY()[2]);
        nodeItem->setSendPositionChanges(true);
        QObject::connect(nodeItem, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onNodePositionChanged2(QPointF)));
    }

    interpolationItem = new GraphicsInterpolationItem(&interpolator, 0.01, 0, 1, rect.width(), -rect.height(), this);
    interpolationItem->setPen(QPen(QBrush(Qt::black), 2));
    interpolationItem->setPos(rect.bottomLeft());
}

void CubicSplineWaveShapingGraphicsItem::onNodePositionChanged1(QPointF position)
{
    if ((position.x() > interpolator.getX()[0]) && (position.x() < interpolator.getX()[2])) {
        // compute the new spline parameters:
        QVector<double> xx = interpolator.getX(), yy = interpolator.getY();
        xx[1] = position.x();
        yy[1] = position.y();
        interpolator = CubicSplineInterpolator(xx, yy);
        // update the spline graphics:
        interpolationItem->updatePath();
        // send the changes to the associated client:
        CubicSplineWaveShapingParameters parameters;
        parameters.x[0] = xx[0];
        parameters.y[0] = yy[0];
        parameters.y2[0] = interpolator.getY2()[0];
        parameters.x[1] = xx[1];
        parameters.y[1] = yy[1];
        parameters.y2[1] = interpolator.getY2()[1];
        parameters.x[2] = xx[2];
        parameters.y[2] = yy[2];
        parameters.y2[2] = interpolator.getY2()[2];
        parameters.x[3] = xx[3];
        parameters.y[3] = yy[3];
        parameters.y2[3] = interpolator.getY2()[3];
        client->postEvent(parameters);
    }
}

void CubicSplineWaveShapingGraphicsItem::onNodePositionChanged2(QPointF position)
{
    if ((position.x() > interpolator.getX()[1]) && (position.x() < interpolator.getX()[3])) {
        // compute the new spline parameters:
        QVector<double> xx = interpolator.getX(), yy = interpolator.getY();
        xx[2] = position.x();
        yy[2] = position.y();
        interpolator = CubicSplineInterpolator(xx, yy);
        // update the spline graphics:
        interpolationItem->updatePath();
        // send the changes to the associated client:
        CubicSplineWaveShapingParameters parameters;
        parameters.x[0] = xx[0];
        parameters.y[0] = yy[0];
        parameters.y2[0] = interpolator.getY2()[0];
        parameters.x[1] = xx[1];
        parameters.y[1] = yy[1];
        parameters.y2[1] = interpolator.getY2()[1];
        parameters.x[2] = xx[2];
        parameters.y[2] = yy[2];
        parameters.y2[2] = interpolator.getY2()[2];
        parameters.x[3] = xx[3];
        parameters.y[3] = yy[3];
        parameters.y2[3] = interpolator.getY2()[3];
        client->postEvent(parameters);
    }
}
