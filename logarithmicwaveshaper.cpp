#include "logarithmicwaveshaper.h"

#include "graphicsnodeitem.h"
#include <QPen>
#include <QtGlobal>
#include <cmath>

LogarithmicWaveShaper::LogarithmicWaveShaper() :
    AudioProcessor(QStringList("Audio in"), QStringList("Audio out")),
    LogarithmicInterpolator(1)
{
    QVector<double> xx, yy;
    xx.append(-1);
    yy.append(-1);
    xx.append(1);
    yy.append(1);
    changeControlPoints(xx, yy);
    // register numeric parameters:
    registerParameter("Slope", 0, -5, 5, 0.1);
    registerParameter("X steps", 0, 0, 16, 1);
    registerParameter("Y steps", 0, 0, 12, 1);
}

void LogarithmicWaveShaper::addControlPoint(double x, double y)
{
    double xSteps = getParameter(1).value;
    double ySteps = getParameter(2).value;
    if (xSteps) {
        // discretize the X coordinate:
        x = qRound(x * xSteps) / xSteps;
    }
    if (ySteps) {
        // discretize the Y coordinate:
        y = qRound(y * ySteps) / ySteps;
    }
    LogarithmicInterpolator::addControlPoint(x, y);
}

void LogarithmicWaveShaper::changeControlPoint(int index, double x, double y)
{
    double xSteps = getParameter(1).value;
    double ySteps = getParameter(2).value;
    if (xSteps) {
        // discretize the X coordinate:
        x = qRound(x * xSteps) / xSteps;
    }
    if (ySteps) {
        // discretize the Y coordinate:
        y = qRound(y * ySteps) / ySteps;
    }
    LogarithmicInterpolator::changeControlPoint(index, x, y);
}

void LogarithmicWaveShaper::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    outputs[0] = evaluate(inputs[0]);
}

bool LogarithmicWaveShaper::processEvent(const RingBufferEvent *event, jack_nframes_t)
{
    if (const Interpolator::InterpolatorEvent *event_ = dynamic_cast<const Interpolator::InterpolatorEvent*>(event)) {
        processInterpolatorEvent(event_);
        return true;
    } else {
        return false;
    }
}

bool LogarithmicWaveShaper::setParameterValue(int index, double value, double min, double max, unsigned int time)
{
    if (ParameterProcessor::setParameterValue(index, value, min, max, time)) {
        const ParameterProcessor::Parameter &parameter = getParameter(index);
        if (index == 0) {
            // slope:
            LogarithmicInterpolator::setBase(pow(1000.0, parameter.value));
        }
        return true;
    } else {
        return false;
    }
}

LogarithmicWaveShapingClient::LogarithmicWaveShapingClient(const QString &clientName, LogarithmicWaveShaper *processWaveShaper_, LogarithmicWaveShaper * guiWaveShaper_, size_t ringBufferSize) :
    ParameterClient(clientName, processWaveShaper_, 0, processWaveShaper_, processWaveShaper_, guiWaveShaper_, ringBufferSize),
    processWaveShaper(processWaveShaper_),
    guiWaveShaper(guiWaveShaper_)
{
}

LogarithmicWaveShapingClient::~LogarithmicWaveShapingClient()
{
    close();
    delete processWaveShaper;
    delete guiWaveShaper;
}

void LogarithmicWaveShapingClient::saveState(QDataStream &stream)
{
    EventProcessorClient::saveState(stream);
    guiWaveShaper->save(stream);
}

void LogarithmicWaveShapingClient::loadState(QDataStream &stream)
{
    EventProcessorClient::loadState(stream);
    guiWaveShaper->load(stream);
    processWaveShaper->changeControlPoints(guiWaveShaper->getX(), guiWaveShaper->getY());
}

QGraphicsItem * LogarithmicWaveShapingClient::createGraphicsItem()
{
    int padding = 4;
    QGraphicsRectItem *item = new QGraphicsRectItem();
    QGraphicsItem *parameterItem = ParameterClient::createGraphicsItem();
    QRectF rect = QRect(0, 0, 420, 420);
    rect = rect.translated(parameterItem->boundingRect().width() + 2 * padding, padding);
    parameterItem->setPos(padding, padding);
    parameterItem->setParentItem(item);
    GraphicsInterpolatorEditItem *ourItem = new GraphicsInterpolatorEditItem(this, rect, QRectF(-1, 1, 2, -2), item);
    item->setRect((rect | parameterItem->boundingRect().translated(parameterItem->pos())).adjusted(-padding, -padding, padding, padding));
    item->setPen(QPen(QBrush(Qt::black), 1));
    item->setBrush(QBrush(Qt::white));
    QObject::connect(this, SIGNAL(changedXSteps(int)), ourItem, SLOT(setVerticalSlices(int)));
    QObject::connect(this, SIGNAL(changedYSteps(int)), ourItem, SLOT(setHorizontalSlices(int)));
    QObject::connect(this, SIGNAL(changedParameterValue(int,double,double,double)), ourItem, SLOT(updateInterpolator()));
    return item;
}

double LogarithmicWaveShapingClient::evaluate(double x, int *index)
{
    return guiWaveShaper->evaluate(x, index);
}

int LogarithmicWaveShapingClient::getNrOfControlPoints()
{
    return guiWaveShaper->getNrOfControlPoints();
}

QPointF LogarithmicWaveShapingClient::getControlPoint(int index)
{
    return guiWaveShaper->getControlPoint(index);
}

void LogarithmicWaveShapingClient::changeControlPoint(int index, double x, double y)
{
    guiWaveShaper->changeControlPoint(index, x, y);
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent(index, x, y);
    postEvent(event);
}

void LogarithmicWaveShapingClient::addControlPoint(double x, double y)
{
    guiWaveShaper->addControlPoint(x, y);
    Interpolator::AddControlPointEvent *event = new Interpolator::AddControlPointEvent(x, y);
    postEvent(event);
}

void LogarithmicWaveShapingClient::deleteControlPoint(int index)
{
    guiWaveShaper->deleteControlPoint(index);
    Interpolator::DeleteControlPointEvent *event = new Interpolator::DeleteControlPointEvent(index);
    postEvent(event);
}

QString LogarithmicWaveShapingClient::getControlPointName(int index) const
{
    return guiWaveShaper->getControlPointName(index);
}

void LogarithmicWaveShapingClient::onChangedParameterValue(int index, double value, double min, double max)
{
    if (index == 1) {
        // x steps:
        changedXSteps(qRound(value));
    } else if (index == 2) {
        // y steps:
        changedYSteps(qRound(value));
    }
    ParameterClient::onChangedParameterValue(index, value, min, max);
}

class LogarithmicWaveShapingClientFactory : public JackClientFactory
{
public:
    LogarithmicWaveShapingClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Logarithmic shaper";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new LogarithmicWaveShapingClient(clientName, new LogarithmicWaveShaper(), new LogarithmicWaveShaper());
    }
    static LogarithmicWaveShapingClientFactory factory;
};

LogarithmicWaveShapingClientFactory LogarithmicWaveShapingClientFactory::factory;

JackClientFactory * LogarithmicWaveShapingClient::getFactory()
{
    return &LogarithmicWaveShapingClientFactory::factory;
}
