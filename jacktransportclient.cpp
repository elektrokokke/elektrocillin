#include "jacktransportclient.h"
#include "graphicsclientitem.h"

JackTransportClient::JackTransportClient(const QString &clientName, size_t ringBufferSize) :
    JackThreadEventProcessorClient(new JackTransportThread(this), clientName, QStringList(), QStringList(), ringBufferSize),
    ringBufferToThread(ringBufferSize),
    beatsPerMinute(120),
    beatsPerBar(4),
    beatType(4),
    ticksPerBeat(1920)
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

bool JackTransportClient::init()
{
    // register a jack timebase callback:
    jack_set_timebase_callback(getClient(), 0, timebase, this);
    return JackThreadEventProcessorClient::init();
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

void JackTransportClient::timebase(jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos)
{
    // we provide the JackPositionBBT fields (and the recommended frame offset):
    pos->valid = (jack_position_bits_t)(JackPositionBBT | JackBBTFrameOffset);
    pos->beats_per_bar = beatsPerBar;
    pos->beat_type = beatType;
    pos->ticks_per_beat = ticksPerBeat;
    pos->beats_per_minute = beatsPerMinute;
    // compute current bar, beat and tick from the current frame time, the frame rate and the beats per minute:
    double timeInMinutes = (double)pos->frame / ((double)pos->frame_rate * 60.0);
    double beat = timeInMinutes * beatsPerMinute;
    double tick = beat * (double)ticksPerBeat;
    double bar = beat / (double)beatsPerBar;
    // compute what exact frame time the rounded-down bar/beat/tick corresponds to:
    // tick = ticksPerBeat * beatsPerMinute * frame / (frame_rate * 60);
    // => frame = tick * frame_rate * 60 / (ticksPerBeat * beatsPerMinute)
    pos->tick = (int)tick;
    jack_nframes_t frame = (int)((double)tick * (double)pos->frame_rate * 60.0 / ((double)ticksPerBeat * beatsPerMinute));
    Q_ASSERT(frame <= pos->frame);
    // save the difference in the frame offset field:
    pos->bbt_offset = pos->frame - frame;
    pos->tick = pos->tick % ticksPerBeat;
    pos->beat = (int)beat % beatsPerBar + 1;
    pos->bar = (int)bar + 1;
    pos->bar_start_tick = (double)(pos->bar - 1) * (double)beatsPerBar * (double)ticksPerBeat;
}

void JackTransportClient::timebase(jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos, void *arg)
{
    JackTransportClient *client = (JackTransportClient*)arg;
    client->timebase(state, nframes, pos, new_pos);
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
    GraphicsLabelItem(parent)
{
}

void JackTransportGraphicsItem::changePosition(const QString &pos)
{
    setText(pos);
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
