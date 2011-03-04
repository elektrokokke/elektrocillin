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
#include <jack/ringbuffer.h>

//class Record2MemoryClient : public QThread, public JackClient
//{
//    Q_OBJECT
//public:
//    Record2MemoryClient(const QString &clientName, QObject *parent = 0);
//    virtual ~Record2MemoryClient();

//    int getNrOfAudioModels();
//    JackAudioModel * removeAudioModel(int i);
//    JackAudioModel * popAudioModel();

//    QGraphicsItem * createGraphicsItem(const QRectF &rect);

//signals:
//    void recordingStarted();
//    void recordingFinished();

//protected:
//    // reimplemented methods from JackClient:
//    virtual bool init();
//    virtual void deinit();
//    virtual bool process(jack_nframes_t nframes);
//    // reimplemented methods from QThread:
//    virtual void run();

//private:
//    // use a lock-free ring buffer for communication between threads:
//    jack_ringbuffer_t *ringBuffer, *ringBufferStopThread;
//    // provide an audio and a midi input port:
//    jack_port_t *audioIn, *midiIn;
//    // wait condition to wait for audio to record:
//    QWaitCondition waitForAudio;
//    // corresponding mutex:
//    QMutex mutexForAudio;
//    // this variable is accessed only from the process() method!
//    bool isRecording_process;
//    // these, too:
//    unsigned char channelRecordTrigger_process, noteRecordTrigger_process;
//    // this variable is only access from the QThread run() method, until recording is finished!
//    JackAudioModel *audioModel_run;
//    // this variable is accessed from anywhere, but only by using the below mutex for synchronization:
//    QVector<JackAudioModel*> audioModels;
//    // mutex for accessing the vector of audio models:
//    QMutex audioModelsMutex;

//    static const size_t ringBufferSize;
//};

//class Record2MemoryGraphicsItem : public QObject, public QGraphicsRectItem
//{
//    Q_OBJECT
//public:
//    Record2MemoryGraphicsItem(const QRectF &rect, Record2MemoryClient *client, QGraphicsItem *parent = 0);

//    GraphView * getGraphView();

//public slots:
//    void resizeForView(QGraphicsView *view);

//private:
//    Record2MemoryClient *client;
//    GraphView *recordClientGraphView;
//};

#endif // RECORD2MEMORYCLIENT_H
