#ifndef MIDICLIENT_H
#define MIDICLIENT_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QByteArray>
#include "midiprocessorclient.h"
#include "jackthread.h"
#include "jackclientwithdeferredprocessing.h"
#include "jack/midiport.h"

struct MidiEventWithTimeStamp {
    jack_nframes_t time;
    MidiProcessorClient::MidiEvent event;
};

class MidiSignalThread : public JackThread
{
    Q_OBJECT
public:
    explicit MidiSignalThread(const QString &clientName, QObject *parent = 0);

    JackRingBuffer<MidiEventWithTimeStamp> * getInputRingBuffer();
    JackRingBuffer<MidiEventWithTimeStamp> * getOutputRingBuffer();

protected:
    // reimplemented methods from JackThread:
    void processDeferred();

signals:
    // "voice" message signals:
    void receivedNoteOff(unsigned char channel, unsigned char note, unsigned char velocity);
    void receivedNoteOn(unsigned char channel, unsigned char note, unsigned char velocity);
    void receivedAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure);
    void receivedControlChange(unsigned char channel, unsigned char controller, unsigned char value);
    void receivedProgramChange(unsigned char channel, unsigned char program);
    void receivedChannelPressure(unsigned char channel, unsigned char pressure);
    void receivedPitchWheel(unsigned char channel, unsigned int pitch);

public slots:
    // "voice" message slots:
    void sendNoteOff(unsigned char channel, unsigned char note, unsigned char velocity);
    void sendNoteOn(unsigned char channel, unsigned char note, unsigned char velocity);
    void sendAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure);
    void sendControlChange(unsigned char channel, unsigned char controller, unsigned char value);
    void sendProgramChange(unsigned char channel, unsigned char program);
    void sendChannelPressure(unsigned char channel, unsigned char pressure);
    void sendPitchWheel(unsigned char channel, unsigned int pitch);

private:
    class MidiSignalClient : public JackClientWithDeferredProcessing
    {
    public:
        MidiSignalClient(const QString &clientName, JackThread *thread);
        virtual ~MidiSignalClient();

        const QString & getMidiInputPortName() const;
        const QString & getMidiOutputPortName() const;
        MidiSignalThread * getMidiThread();

    protected:
        JackRingBuffer<MidiEventWithTimeStamp> * getInputRingBuffer();
        JackRingBuffer<MidiEventWithTimeStamp> * getOutputRingBuffer();
        // reimplemented methods from JackClient:
        virtual bool init();
        virtual bool process(jack_nframes_t nframes);

    private:
        QString midiInputPortName, midiOutputPortName;
        // provide one midi in- and output:
        jack_port_t *midiIn, *midiOut;
    };

    MidiSignalClient client;
    // use lock-free ring buffers for communication between threads:
    JackRingBuffer<MidiEventWithTimeStamp> ringBufferFromClient, ringBufferToClient;

};

#endif // MIDICLIENT_H
