#include "record2memoryclient.h"
#include "metajack/midiport.h"
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>

const size_t Record2MemoryClient::ringBufferSize = 2 << 20;

Record2MemoryClient::Record2MemoryClient(const QString &clientName) :
    JackClient(clientName),
    isRecording_process(false)
{
    // create the ring buffers:
    ringBuffer = jack_ringbuffer_create(ringBufferSize);
    ringBufferStopThread = jack_ringbuffer_create(1);
    // create the associated thread:
    thread = new Record2MemoryThread(this, ringBuffer, ringBufferStopThread);
}

Record2MemoryClient::~Record2MemoryClient()
{
    close();
    // delete the ring buffer:
    jack_ringbuffer_free(ringBuffer);
    jack_ringbuffer_free(ringBufferStopThread);
}

QGraphicsItem * Record2MemoryClient::createGraphicsItem()
{
    return new Record2MemoryGraphicsItem(QRectF(0, 0, 600, 420), this);
}

Record2MemoryThread * Record2MemoryClient::getThread()
{
    return thread;
}

bool Record2MemoryClient::init()
{
    // start the QThread:
    if (!thread->isRunning()) {
        thread->start();
    }
    isRecording_process = false;
    // setup the audio and midi input ports:
    audioIn = registerAudioPort("Audio in", JackPortIsInput);
    midiIn = registerMidiPort("Midi in", JackPortIsInput);
    return (ringBuffer && audioIn && midiIn);
}

void Record2MemoryClient::deinit()
{
    // tell the QThread to shut down:
    char dummy = 0;
    jack_ringbuffer_write(ringBufferStopThread, &dummy, 1);
    thread->wake();
    // wait for the thread to finish:
    thread->wait();
}

bool Record2MemoryClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    jack_default_audio_sample_t *audioBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(audioIn, nframes);
    void *midiInputBuffer = jack_port_get_buffer(midiIn, nframes);
    jack_nframes_t recordingStart = 0, recordingStop = nframes;
    jack_nframes_t midiInCount = jack_midi_get_event_count(midiInputBuffer);
    for (jack_nframes_t midiInIndex = 0; midiInIndex < midiInCount; midiInIndex++) {
        // read from midi input port and wait for record signals (note on or off):
        jack_midi_event_t midiEvent;
        jack_midi_event_get(&midiEvent, midiInputBuffer, midiInIndex);
        // look explicitly for note on and off messages:
        if (midiEvent.size == 3) {
            bool noteOn = false, noteOff = false;
            if ((midiEvent.buffer[0] >> 4) == 0x09) {
                noteOn = midiEvent.buffer[2];
                noteOff = !noteOn;
            } else if ((midiEvent.buffer[0] >> 4) == 0x08) {
                noteOff = true;
            }
            if (!isRecording_process) {
                // if we are not recording we look for note on messages with non-zero velocity:
                if (noteOn) {
                    isRecording_process = true;
                    recordingStart = midiEvent.time;
                    // remember on which channel and which note the trigger was:
                    channelRecordTrigger_process = midiEvent.buffer[0] & 0x0F;
                    noteRecordTrigger_process = midiEvent.buffer[1];
                }
            } else {
                // otherwise we look for note off messages or note on messages with zero velocity:
                // also we look only for the trigger channel and note number...
                if (noteOff && ((midiEvent.buffer[0] & 0x0F) == channelRecordTrigger_process) && (midiEvent.buffer[1] == noteRecordTrigger_process)) {
                    isRecording_process = false;
                    recordingStop = midiEvent.time;
                    // do the recording for the given segment:
                    // (record starting with frame at recordingStart, and ends with recordingStop)
                    // i.e., put all frames starting with that into the ring buffer:
                    jack_nframes_t framesToWrite = recordingStop - recordingStart;
                    size_t bytesToWrite = framesToWrite * sizeof(jack_default_audio_sample_t);
                    if (jack_ringbuffer_write_space(ringBuffer) >= sizeof(jack_nframes_t) + bytesToWrite + sizeof(jack_nframes_t)) {
                        // tell the QThread how many frames there are:
                        jack_ringbuffer_write(ringBuffer, (const char*)&framesToWrite, sizeof(jack_nframes_t));
                        // write the frames:
                        jack_ringbuffer_write(ringBuffer, (const char*)(audioBuffer + recordingStart), bytesToWrite);
                        // tell the QThread to also stop recording:
                        framesToWrite = 0;
                        jack_ringbuffer_write(ringBuffer, (const char*)&framesToWrite, sizeof(jack_nframes_t));
                        // tell the QThread to wake up and read from the ring buffer:
                        thread->wake();
                    }
                }
            }
        }
    }
    // do all recording that isn't done yet:
    if (isRecording_process) {
        jack_nframes_t framesToWrite = recordingStop - recordingStart;
        size_t bytesToWrite = framesToWrite * sizeof(jack_default_audio_sample_t);
        if (jack_ringbuffer_write_space(ringBuffer) >= sizeof(jack_nframes_t) + bytesToWrite) {
            // tell the QThread how many frames there are:
            jack_ringbuffer_write(ringBuffer, (const char*)&framesToWrite, sizeof(jack_nframes_t));
            // write the frames:
            jack_ringbuffer_write(ringBuffer, (const char*)(audioBuffer + recordingStart), bytesToWrite);
            // tell the QThread to wake up and read from the ring buffer:
            thread->wake();
        }
    }
    return true;
}

Record2MemoryThread::Record2MemoryThread(Record2MemoryClient *client_, jack_ringbuffer_t *ringBuffer_, jack_ringbuffer_t *ringBufferStopThread_, QObject *parent) :
    QThread(parent),
    client(client_),
    ringBuffer(ringBuffer_),
    ringBufferStopThread(ringBufferStopThread_),
    audioModel_run(0)
{
}

int Record2MemoryThread::getNrOfAudioModels()
{
    // lock the mutex first:
    audioModelsMutex.lock();
    // get the information:
    int size = audioModels.size();
    // free the mutex:
    audioModelsMutex.unlock();
    // return the information:
    return size;
}

JackAudioModel * Record2MemoryThread::removeAudioModel(int i)
{
    // lock the mutex first:
    audioModelsMutex.lock();
    // get the audio model:
    JackAudioModel *model = audioModels[i];
    // remove it from the vector:
    audioModels.remove(i);
    // free the mutex:
    audioModelsMutex.unlock();
    // return the model:
    return model;
}

JackAudioModel * Record2MemoryThread::popAudioModel()
{
    // lock the mutex first:
    audioModelsMutex.lock();
    // pop the last audio model:
    JackAudioModel *model = 0;
    if (audioModels.size()) {
        model = audioModels.last();
        audioModels.remove(audioModels.size() - 1);
    }
    // free the mutex:
    audioModelsMutex.unlock();
    // return the model:
    return model;
}

void Record2MemoryThread::wake()
{
    waitForAudio.wakeOne();
}

Record2MemoryClient * Record2MemoryThread::getClient()
{
    return client;
}

void Record2MemoryThread::run()
{
    // mutex has to be locked to be used for the wait condition:
    mutexForAudio.lock();
    for (; jack_ringbuffer_read_space(ringBufferStopThread) == 0; ) {
        // wait for audio to record:
        waitForAudio.wait(&mutexForAudio);
        // read how many frames there are in the ring buffer:
        for (; jack_ringbuffer_read_space(ringBuffer) >= sizeof(jack_nframes_t); ) {
            jack_nframes_t framesToRead = 0;
            jack_ringbuffer_peek(ringBuffer, (char*)&framesToRead, sizeof(jack_nframes_t));
            if (framesToRead == 0) {
                jack_ringbuffer_read_advance(ringBuffer, sizeof(jack_nframes_t));
                // recording has stopped:
                if (audioModel_run) {
                    // push the model into the application thread:
                    audioModel_run->moveToThread(qApp->thread());
                    // lock the mutex first:
                    audioModelsMutex.lock();
                    // put the model in the vector:
                    audioModels.append(audioModel_run);
                    // free the mutex:
                    audioModelsMutex.unlock();
                    audioModel_run = 0;
                    // invoke the corresponding signal:
                    recordingFinished();
                }
            } else if (jack_ringbuffer_read_space(ringBuffer) >= sizeof(jack_nframes_t) + framesToRead * sizeof(jack_default_audio_sample_t)) {
                if (!audioModel_run) {
                    recordingStarted();
                    // create the audio model:
                    audioModel_run = new JackAudioModel();
                    // create a column in the audio model:
                    audioModel_run->insertColumn(0);
                }
                jack_ringbuffer_read_advance(ringBuffer, sizeof(jack_nframes_t));
                // read audio data from the ring buffer and put it into our audio model:
                audioModel_run->appendRowsFromRingBuffer(framesToRead, ringBuffer);
            }
        }
    }
    char dummy;
    jack_ringbuffer_read(ringBufferStopThread, &dummy, 1);
}

Record2MemoryGraphicsItem::Record2MemoryGraphicsItem(const QRectF &rect, Record2MemoryClient *client_, QGraphicsItem *parent) :
    QGraphicsRectItem(rect, parent),
    client(client_)
{
    GraphicsNodeItem *node = new GraphicsNodeItem(-5, -5, 10, 10, this);
    node->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    node->setBrush(QBrush(qRgb(52, 101, 164)));
    node->setZValue(1);
    node->setBounds(QRectF(rect.left(), 0, rect.width(), 0));
    node->setBoundsScaled(QRectF(0, 0, 10, 0));
    QObject::connect(node, SIGNAL(xChangedScaled(qreal)), this, SLOT(onZoomNode(qreal)));
    recordClientGraphView = new GraphView(0);
    recordClientGraphView->resize(rect.width(), rect.height());
    QGraphicsProxyWidget *recordClientGraphicsItem = new QGraphicsProxyWidget(this);
    recordClientGraphicsItem->setWidget(recordClientGraphView);
    QObject::connect(client->getThread(), SIGNAL(recordingFinished()), this, SLOT(onRecordingFinished()));
}

GraphView * Record2MemoryGraphicsItem::getGraphView()
{
    return recordClientGraphView;
}

void Record2MemoryGraphicsItem::onRecordingFinished()
{
    // get the audio model from the record thread:
    JackAudioModel *model = client->getThread()->popAudioModel();
    recordClientGraphView->setModel(model);
}

void Record2MemoryGraphicsItem::onZoomNode(qreal x)
{
    int zoom = qRound(x);
    recordClientGraphView->setHorizontalScale(zoom);
}

class Record2MemoryClientFactory : public JackClientFactory
{
public:
    Record2MemoryClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Record";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new Record2MemoryClient(clientName);
    }
    static Record2MemoryClientFactory factory;
};

Record2MemoryClientFactory Record2MemoryClientFactory::factory;

JackClientFactory * Record2MemoryClient::getFactory()
{
    return &Record2MemoryClientFactory::factory;
}
