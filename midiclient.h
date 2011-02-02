#ifndef MIDICLIENT_H
#define MIDICLIENT_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QByteArray>
#include "jackthread.h"
#include "jackclientwithdeferredprocessing.h"
#include "jack/midiport.h"

struct MidiMessage {
    jack_nframes_t time, bufferTime;
    size_t size;
    jack_midi_data_t message[3];
};

class MidiThread : public JackThread
{
    Q_OBJECT
public:
    explicit MidiThread(JackClientWithDeferredProcessing *client, QObject *parent = 0);

    JackRingBuffer<MidiMessage> * getInputRingBuffer();
    JackRingBuffer<MidiMessage> * getOutputRingBuffer();

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
    // use lock-free ring buffers for communication between threads:
    JackRingBuffer<MidiMessage> ringBufferFromClient, ringBufferToClient;
};

class MidiClient : public JackClientWithDeferredProcessing
{
public:
    MidiClient(const QString &clientName);
    virtual ~MidiClient();

    const QString & getMidiInputPortName() const;
    const QString & getMidiOutputPortName() const;
    MidiThread * getMidiThread();

protected:
    JackRingBuffer<MidiMessage> * getInputRingBuffer();
    JackRingBuffer<MidiMessage> * getOutputRingBuffer();
    // reimplemented methods from JackClient:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

private:
    QString midiInputPortName, midiOutputPortName;
    // provide one midi in- and output:
    jack_port_t *midiIn, *midiOut;
    MidiThread thread;
};

#endif // MIDICLIENT_H
