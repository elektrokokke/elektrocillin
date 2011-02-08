#include "record2memoryclient.h"
#include "jack/midiport.h"
#include <QApplication>
#include <QDebug>

const size_t Record2MemoryClient::ringBufferSize = 2 << 20;

Record2MemoryClient::Record2MemoryClient(const QString &clientName, QObject *parent) :
    QThread(parent),
    JackClient(clientName),
    isRecording_process(false),
    audioModel_run(0)
{
    // create the ring buffers:
    ringBuffer = jack_ringbuffer_create(ringBufferSize);
    ringBufferStopThread = jack_ringbuffer_create(1);
}

Record2MemoryClient::~Record2MemoryClient()
{
    close();
    // delete the ring buffer:
    if (ringBuffer) {
        jack_ringbuffer_free(ringBuffer);
    }
}

int Record2MemoryClient::getNrOfAudioModels()
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

JackAudioModel * Record2MemoryClient::removeAudioModel(int i)
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

JackAudioModel * Record2MemoryClient::popAudioModel()
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

bool Record2MemoryClient::init()
{
    // start the QThread:
    if (!isRunning()) {
        start();
    }
    isRecording_process = false;
    // setup the audio and midi input ports:
    audioIn = registerAudioPort("audio in", JackPortIsInput);
    midiIn = registerMidiPort("midi in", JackPortIsInput);
    return (ringBuffer && audioIn && midiIn);
}

void Record2MemoryClient::deinit()
{
    // tell the QThread to shut down:
    char dummy = 0;
    jack_ringbuffer_write(ringBufferStopThread, &dummy, 1);
    waitForAudio.wakeOne();
    // wait for the thread:
    wait();
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
                        waitForAudio.wakeOne();
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
            waitForAudio.wakeOne();
        }
    }
    return true;
}

void Record2MemoryClient::run()
{
    qDebug() << "Record2MemoryClient::run() : starting thread";
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
                    qDebug() << "recordingFinished();";
                    recordingFinished();
                }
            } else if (jack_ringbuffer_read_space(ringBuffer) >= sizeof(jack_nframes_t) + framesToRead * sizeof(jack_default_audio_sample_t)) {
                if (!audioModel_run) {
                    qDebug() << "recordingStarted();";
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
    qDebug() << "Record2MemoryClient::run() : shutting down thread";
    char dummy;
    jack_ringbuffer_read(ringBufferStopThread, &dummy, 1);
}
