#ifndef RECORD2MEMORYCLIENT_H
#define RECORD2MEMORYCLIENT_H

#include <QThread>
#include <QVector>
#include <QWaitCondition>
#include <QMutex>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include "jackclient.h"
#include "jackaudiomodel.h"
#include "graphview.h"
#include "graphicsnodeitem.h"
#include <jack/ringbuffer.h>

class Record2MemoryThread;

class Record2MemoryClient : public JackClient
{
    Q_OBJECT
public:
    Record2MemoryClient(const QString &clientName);
    virtual ~Record2MemoryClient();
    QGraphicsItem * createGraphicsItem();
    Record2MemoryThread * getThread();
    JackClientFactory * getFactory();
protected:
    // reimplemented methods from JackClient:
    virtual bool init();
    virtual void deinit();
    virtual bool process(jack_nframes_t nframes);
private:
    // use a lock-free ring buffer for communication between threads:
    jack_ringbuffer_t *ringBuffer, *ringBufferStopThread;
    Record2MemoryThread *thread;
    // provide an audio and a midi input port:
    jack_port_t *audioIn, *midiIn;
    // this variable is to be accessed only from the process() method!
    bool isRecording_process;
    // these, too:
    unsigned char channelRecordTrigger_process, noteRecordTrigger_process;

    static const size_t ringBufferSize;
};

class Record2MemoryThread : public QThread
{
    Q_OBJECT
public:
    Record2MemoryThread(Record2MemoryClient *client, jack_ringbuffer_t *ringBuffer, jack_ringbuffer_t *ringBufferStopThread, QObject *parent = 0);

    int getNrOfAudioModels();
    JackAudioModel * removeAudioModel(int i);
    JackAudioModel * popAudioModel();
    void wake();
    Record2MemoryClient * getClient();
signals:
    void recordingStarted();
    void recordingFinished();
protected:
    // reimplemented method from QThread:
    virtual void run();
private:
    Record2MemoryClient *client;
    jack_ringbuffer_t *ringBuffer, *ringBufferStopThread;
    // wait condition to wait for audio to record:
    QWaitCondition waitForAudio;
    // corresponding mutex:
    QMutex mutexForAudio;
    // this variable is to be accessed only from the QThread run() method, until recording is finished!
    JackAudioModel *audioModel_run;
    // this variable is accessed from anywhere, but only by using the below mutex for synchronization:
    QVector<JackAudioModel*> audioModels;
    // mutex for accessing the vector of audio models:
    QMutex audioModelsMutex;
};

class Record2MemoryGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    Record2MemoryGraphicsItem(const QRectF &rect, Record2MemoryClient *client, QGraphicsItem *parent = 0);

    GraphView * getGraphView();

private slots:
    void onRecordingFinished();
    void onZoomNode(qreal x);

private:
    Record2MemoryClient *client;
    GraphView *recordClientGraphView;
};

#endif // RECORD2MEMORYCLIENT_H
