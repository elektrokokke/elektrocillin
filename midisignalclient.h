#ifndef MIDICLIENT_H
#define MIDICLIENT_H

#include "midiprocessorclient.h"
#include "jackthreadeventprocessorclient.h"
#include "jackclientwithdeferredprocessing.h"
#include "graphicskeyboarditem.h"

class MidiSignalClient;

class MidiSignalThread : public JackThread {
    Q_OBJECT
public:
    MidiSignalThread(MidiSignalClient *client, QObject *parent = 0);

    MidiSignalClient *getMidiSignalClient();

    void setRingBufferFromClient(JackRingBuffer<MidiProcessorClient::MidiEvent> *ringBufferFromClient);
signals:
    void receivedNoteOff(unsigned char channel, unsigned char note, unsigned char velocity);
    void receivedNoteOn(unsigned char channel, unsigned char note, unsigned char velocity);
    void receivedAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure);
    void receivedControlChange(unsigned char channel, unsigned char controller, unsigned char value);
    void receivedProgramChange(unsigned char channel, unsigned char program);
    void receivedChannelPressure(unsigned char channel, unsigned char pressure);
    void receivedPitchWheel(unsigned char channel, unsigned int pitch);
public slots:
    void sendNoteOff(unsigned char channel, unsigned char note, unsigned char velocity);
    void sendNoteOn(unsigned char channel, unsigned char note, unsigned char velocity);
    void sendAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure);
    void sendControlChange(unsigned char channel, unsigned char controller, unsigned char value);
    void sendProgramChange(unsigned char channel, unsigned char program);
    void sendChannelPressure(unsigned char channel, unsigned char pressure);
    void sendPitchWheel(unsigned char channel, unsigned int pitch);
protected:
    void processDeferred();
private:
    JackRingBuffer<MidiProcessorClient::MidiEvent> *ringBufferFromClient;
};

class MidiSignalClient : public JackThreadEventProcessorClient<MidiProcessorClient::MidiEvent> {
public:
    MidiSignalClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~MidiSignalClient();

    MidiSignalThread * getMidiSignalThread();

    QGraphicsItem * createGraphicsItem(const QRectF &rect);

protected:
    // reimplemented from EventProcessor:
    virtual void processEvent(const MidiProcessorClient::MidiEvent &event, jack_nframes_t time);
    // reimplemented from MidiProcessor:
    virtual void processMidi(const MidiProcessorClient::MidiEvent &event, jack_nframes_t time);

private:
    JackRingBuffer<MidiProcessorClient::MidiEvent> ringBufferToThread;
};

class MidiSignalGraphicsItem : public GraphicsKeyboardItem
{
    Q_OBJECT
public:
    MidiSignalGraphicsItem(const QRectF &rect, MidiSignalClient *client, QGraphicsItem *parent = 0);
};

struct MidiEventWithTimeStamp {
    jack_nframes_t time;
    MidiProcessorClient::MidiEvent event;
};

#endif // MIDICLIENT_H
