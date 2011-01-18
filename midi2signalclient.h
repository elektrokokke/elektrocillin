#ifndef MIDI2SIGNALCLIENT_H
#define MIDI2SIGNALCLIENT_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QByteArray>
#include "jackclient.h"
#include "jack/ringbuffer.h"
#include "jack/midiport.h"

struct MidiMessage {
    jack_nframes_t time;
    size_t size;
    jack_midi_data_t message[3];
};

class Midi2SignalClient : public QThread, public JackClient
{
    Q_OBJECT
public:
    explicit Midi2SignalClient(const QString &clientName, QObject *parent = 0);
    virtual ~Midi2SignalClient();

protected:
    // reimplemented methods from JackClient:
    virtual bool setup();
    virtual bool process(jack_nframes_t nframes);
    // reimplemented methods from QThread:
    virtual void run();

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
    // use lock-free ring buffers for communication with the Jack process thread:
    jack_ringbuffer_t *ringBufferIn, *ringBufferOut;
    // provide one midi in- and output:
    jack_port_t *midiIn, *midiOut;
    // wait condition to wait for midi input:
    QWaitCondition waitForMidi;
    // corresponding mutex:
    QMutex mutexForMidi;

    static const size_t ringBufferSize;

    void writeMidiEventToOutputRingBuffer(const MidiMessage &message);
};

#endif // MIDI2SIGNALCLIENT_H
