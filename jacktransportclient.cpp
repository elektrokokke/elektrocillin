#include "jacktransportclient.h"
#include "graphicsclientitem.h"

JackTransportClient::JackTransportClient(const QString &clientName, size_t ringBufferSize) :
    JackThreadEventProcessorClient(new JackTransportThread(this), clientName, QStringList(), QStringList(), ringBufferSize),
    ringBufferToThread(ringBufferSize),
    lastFrameTime(0),
    currentBeatTime(0),
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

bool JackTransportClient::process(jack_nframes_t nframes)
{
    // get the transport state and send it to the associated thread:
    jack_position_t pos;
    jack_transport_query(getClient(), &pos);
    ringBufferToThread.write(pos);
    wakeJackThread();
    // do the standard processing:
    return JackThreadEventProcessorClient::process(nframes);
}

void JackTransportClient::processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
    // set the bpm from the midi note number:
    beatsPerMinute = noteNumber + 60;
}

void JackTransportClient::processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
}

void JackTransportClient::processAfterTouch(unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time)
{
}

void JackTransportClient::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
}

void JackTransportClient::processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time)
{
}

void JackTransportClient::processChannelPressure(unsigned char channel, unsigned char pressure, jack_nframes_t time)
{
}

bool JackTransportClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const TimebaseEvent *event_ = dynamic_cast<const TimebaseEvent*>(event)) {
        // only allow changing the beat settings when transport is stopped:
        if (jack_transport_query(getClient(), 0) == JackTransportStopped) {
            beatsPerMinute = event_->beatsPerMinute;
            beatsPerBar = event_->beatsPerBar;
            beatType = event_->beatType;
        }
        return true;
    } else {
        return false;
    }
}

void JackTransportClient::timebase(jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos)
{
    // we provide the JackPositionBBT fields (and the recommended frame offset):
    pos->valid = (jack_position_bits_t)(JackPositionBBT | JackBBTFrameOffset);
    pos->beats_per_bar = beatsPerBar;
    pos->beat_type = beatType;
    pos->ticks_per_beat = ticksPerBeat;
    pos->beats_per_minute = beatsPerMinute;
    if (new_pos) {
        if (lastFrameTime + nframes != pos->frame) {
            double frame = (double)pos->frame;
            double framesPerMinute = (double)pos->frame_rate * 60.0;
            double timeInMinutes = frame / framesPerMinute;
            currentBeatTime = timeInMinutes * beatsPerMinute;
        }
    }
    lastFrameTime = pos->frame;
    double tick = currentBeatTime * (double)ticksPerBeat;
    double bar = currentBeatTime / (double)beatsPerBar;
    pos->tick = (int)tick;
    // save the difference between rounded-down tick and floating-point tick in frames in the frame offset field:
    // 1 tick = 1 minute / beatsPerMinute / ticksPerBeat
    // 1 frame = 1 minute / 60 / frame_rate => 1 minute = 1 frame * 60 * frame_rate
    // => 1 tick = 1 frame * 60 * frame_rate / beatsPerMinute / ticksPerBeat
    double tickDifference = tick - (double)pos->tick;
    pos->bbt_offset = (jack_nframes_t)(tickDifference * 60.0 * (double)pos->frame_rate / ((double)beatsPerMinute * (double)ticksPerBeat));
    pos->tick = pos->tick % ticksPerBeat;
    pos->beat = (int)currentBeatTime % beatsPerBar + 1;
    pos->bar = (int)bar + 1;
    pos->bar_start_tick = (double)(pos->bar - 1) * (double)beatsPerBar * (double)ticksPerBeat;
    // compute the next beat time based on the current tempo:
    double incrementInMinutes = (double)nframes / ((double)pos->frame_rate * 60.0);
    currentBeatTime += incrementInMinutes * beatsPerMinute;
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
