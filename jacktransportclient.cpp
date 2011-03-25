#include "jacktransportclient.h"
#include "graphicsclientitem.h"

JackTransportClient::JackTransportClient(const QString &clientName, size_t ringBufferSize) :
    JackThreadEventProcessorClient(new JackTransportThread(this), clientName, QStringList(), QStringList(), ringBufferSize),
    ringBufferToThread(ringBufferSize)
{
    JackTransportThread *thread = (JackTransportThread*)getJackThread();
    thread->setRingBufferFromClient(&ringBufferToThread);
}


JackTransportClient::~JackTransportClient()
{
    // calling close will stop the Jack client and also stop the associated thread:
    close();
    // deleting the associated thread is now safe, as it has been stopped:
    delete getJackThread();
}

QGraphicsItem * JackTransportClient::createGraphicsItem()
{
    JackTransportGraphicsItem *graphicsItem = new JackTransportGraphicsItem();
    JackTransportThread *thread = (JackTransportThread*)getJackThread();
    QObject::connect(thread, SIGNAL(changedPosition(QString)), graphicsItem, SLOT(changePosition(QString)));
    return graphicsItem;
}

bool JackTransportClient::process(jack_nframes_t)
{
    // get the transport state and send it to the associated thread:
    jack_position_t pos;
    jack_transport_query(getClient(), &pos);
    ringBufferToThread.write(pos);
    wakeJackThread();
    return true;
}

JackTransportThread::JackTransportThread(JackTransportClient *client, QObject *parent) :
    JackThread(client, parent),
    ringBufferFromClient(0)
{
}

void JackTransportThread::setRingBufferFromClient(JackRingBuffer<jack_position_t> *ringBufferFromClient)
{
    this->ringBufferFromClient = ringBufferFromClient;
}

void JackTransportThread::processDeferred()
{
    if (ringBufferFromClient && ringBufferFromClient->readSpace()) {
        ringBufferFromClient->readAdvance(ringBufferFromClient->readSpace() - 1);
        jack_position_t pos = ringBufferFromClient->read();
        // create a string describing the current position:
        // (ignore unique_1 and usecs because these are rather unnerving)
        QString posText = QString("frame_rate: %1\nframe: %2")
                .arg(pos.frame_rate)
                .arg(pos.frame);
        if (pos.valid & JackPositionBBT) {
            posText += QString("\nbar: %1\nbeat: %2\ntick: %3\nbar_start_tick: %4\nbeats_per_bar: %5\nbeat_type: %6\nticks_per_beat: %7\nbeats_per_minute: %8")
                    .arg(pos.bar)
                    .arg(pos.beat)
                    .arg(pos.tick)
                    .arg(pos.bar_start_tick)
                    .arg(pos.beats_per_bar)
                    .arg(pos.beat_type)
                    .arg(pos.ticks_per_beat)
                    .arg(pos.beats_per_minute);
        }
        if (pos.valid & JackPositionTimecode) {
            posText += QString("\nframe_time: %1\nnext_time: %2")
                    .arg(pos.frame_time)
                    .arg(pos.next_time);
        }
        if (pos.valid & JackBBTFrameOffset) {
            posText += QString("\nbbt_offset: %1")
                    .arg(pos.bbt_offset);
        }
        changedPosition(posText);
    }
}

JackTransportGraphicsItem::JackTransportGraphicsItem(QGraphicsItem *parent) :
    QGraphicsSimpleTextItem(parent)
{
}

void JackTransportGraphicsItem::changePosition(const QString &pos)
{
    setText(pos);
    if (GraphicsClientItem *clientItem = qgraphicsitem_cast<GraphicsClientItem*>(parentItem())) {
        clientItem->updateBounds();
    }
}

class JackTransportClientFactory : public JackClientFactory
{
public:
    JackTransportClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Transport";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new JackTransportClient(clientName);
    }
    static JackTransportClientFactory factory;
};

JackTransportClientFactory JackTransportClientFactory::factory;

JackClientFactory * JackTransportClient::getFactory()
{
    return &JackTransportClientFactory::factory;
}
