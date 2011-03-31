#include "jacktransportclient.h"
#include "graphicsclientitem.h"
#include "graphicsdiscretecontrolitem.h"
#include <QDebug>

JackTransportClient::JackTransportClient(const QString &clientName, size_t ringBufferSize) :
    JackThreadEventProcessorClient(new JackTransportThread(this), clientName, QStringList(), QStringList("Beat") + QStringList("Bar"), ringBufferSize),
    ringBufferToThread(ringBufferSize),
    lastTransportFrameTime(0),
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
    JackTransportGraphicsItem *graphicsItem = new JackTransportGraphicsItem(this);
    JackTransportThread *thread = (JackTransportThread*)getJackThread();
    QObject::connect(thread, SIGNAL(changedPosition(QString)), graphicsItem, SLOT(changePosition(QString)));
    return graphicsItem;
}

void JackTransportClient::changeBeatsPerMinute(double bpm)
{
    BeatsPerMinuteEvent *event = new BeatsPerMinuteEvent(bpm);
    postEvent(event);
}

void JackTransportClient::changeBeatsPerBar(int beatsPerBar)
{
    BeatsPerBarEvent *event = new BeatsPerBarEvent(beatsPerBar);
    postEvent(event);
}

void JackTransportClient::changeBeatType(int beatType)
{
    BeatTypeEvent *event = new BeatTypeEvent(beatType);
    postEvent(event);
}

bool JackTransportClient::init()
{
    // register a jack timebase callback:
    jack_set_timebase_callback(getClient(), 0, timebase, this);
    return JackThreadEventProcessorClient::init();
}

bool JackTransportClient::process(jack_nframes_t nframes)
{
    // get the current transport state and send it to the associated thread:
    currentState = jack_transport_query(getClient(), &currentPos);
    ringBufferToThread.write(currentPos);
    wakeJackThread();
    // compute a few values which are used in processAudio():
    bbt_offset = (currentPos.valid & JackBBTFrameOffset ? currentPos.bbt_offset : 0);
    framesPerMinute = 60.0 * currentPos.frame_rate;
    ticksPerMinute = (double)currentPos.ticks_per_beat * (double)currentPos.beats_per_minute;
    ticksPerFrame = ticksPerMinute / framesPerMinute;
    // do the standard processing:
    JackThreadEventProcessorClient::process(nframes);
    return true;
}

void JackTransportClient::processAudio(const double *, double *outputs, jack_nframes_t time)
{
    if ((currentPos.valid & JackPositionBBT) && (currentState == JackTransportRolling)) {
        // current tick is bbt_offset frames before the first frame
        double currentTick = (double)(time + bbt_offset) * ticksPerFrame + (double)currentPos.tick;
        // beat position is current tick / ticks per beat:
        double currentBeat = currentTick / (double)currentPos.ticks_per_beat + (double)(currentPos.beat - 1);
        double beatPosition = currentBeat - (int)currentBeat;
        // bar position is current beat / beats per bar:
        double barPosition  = currentBeat / (double)currentPos.beats_per_bar;
        // stretch these value (which lie in [0:1]) to [-1:1]:
        outputs[0] = beatPosition * 2.0 - 1.0;
        outputs[1] = barPosition * 2.0 - 1.0;
    } else {
        outputs[0] = 0;
        outputs[1] = 0;
    }
    outputs[0] = (double)time / (1024);
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
    if (const BeatsPerMinuteEvent *event_ = dynamic_cast<const BeatsPerMinuteEvent*>(event)) {
        beatsPerMinute = event_->beatsPerMinute;
        return true;
    } else if (const BeatsPerBarEvent *event_ = dynamic_cast<const BeatsPerBarEvent*>(event)) {
        beatsPerBar = event_->beatsPerBar;
        return true;
    } else if (const BeatTypeEvent *event_ = dynamic_cast<const BeatTypeEvent*>(event)) {
        beatType = event_->beatType;
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
        if (lastTransportFrameTime + nframes != pos->frame) {
            double frame = (double)pos->frame;
            double framesPerMinute = (double)pos->frame_rate * 60.0;
            double timeInMinutes = frame / framesPerMinute;
            currentBeatTime = timeInMinutes * beatsPerMinute;
        }
    }
    lastTransportFrameTime = pos->frame;
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

JackTransportGraphicsItem::JackTransportGraphicsItem(JackTransportClient *client, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    padding(4)
{
    setPen(QPen(QBrush(Qt::black), 1));
    setBrush(Qt::white);
    labelItem = new GraphicsLabelItem(this);
    GraphicsContinuousControlItem *beatsPerMinuteControl = new GraphicsContinuousControlItem("Beats per minute", 1, 300, 120, 600, GraphicsContinuousControlItem::HORIZONTAL, 'g', -1, 1, this);
    GraphicsDiscreteControlItem *beatsPerBarControl = new GraphicsDiscreteControlItem("Beats per bar", 1, 32, 4, 320, GraphicsContinuousControlItem::HORIZONTAL, this);
    GraphicsDiscreteControlItem *beatTypeControl = new GraphicsDiscreteControlItem("Beat type", 1, 32, 4, 320, GraphicsContinuousControlItem::HORIZONTAL, this);
    beatsPerMinuteControl->setPos(padding, padding + 0 * (padding + beatsPerMinuteControl->rect().height()));
    beatsPerBarControl->setPos(padding, padding + 1 * (padding + beatsPerMinuteControl->rect().height()));
    beatTypeControl->setPos(padding, padding + 2 * (padding + beatsPerMinuteControl->rect().height()));
    controlsRect = (beatsPerMinuteControl->rect().translated(beatsPerMinuteControl->pos()) | beatsPerBarControl->rect().translated(beatsPerBarControl->pos()) | beatTypeControl->rect().translated(beatTypeControl->pos()));
    labelItem->setPos(controlsRect.bottomLeft() + QPointF(0, padding));
    QObject::connect(beatsPerMinuteControl, SIGNAL(valueChanged(double)), client, SLOT(changeBeatsPerMinute(double)));
    QObject::connect(beatsPerBarControl, SIGNAL(valueChanged(int)), client, SLOT(changeBeatsPerBar(int)));
    QObject::connect(beatTypeControl, SIGNAL(valueChanged(int)), client, SLOT(changeBeatType(int)));
    changePosition("");
}

void JackTransportGraphicsItem::changePosition(const QString &pos)
{
    labelItem->setText(pos);
    setRect((controlsRect | labelItem->rect().translated(labelItem->pos())).adjusted(-padding, -padding, padding, padding));
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
