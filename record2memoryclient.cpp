#include "record2memoryclient.h"
#include "jack/midiport.h"
#include <QApplication>

const size_t Record2MemoryClient::ringBufferSize = 2 << 20;

Record2MemoryClient::Record2MemoryClient(const QString &clientName, QObject *parent) :
    QThread(parent),
    JackClient(clientName),
    isRecordingProcess(false),
    audioModelRun(0)
{
    // create the ring buffer:
    ringBuffer = jack_ringbuffer_create(ringBufferSize);
}

Record2MemoryClient::~Record2MemoryClient()
{
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

bool Record2MemoryClient::setup()
{
    // start the QThread:
    if (!isRunning()) {
        start();
    }
    // setup the audio and midi input ports:
    audioIn = registerAudioPort("audio in", JackPortIsInput);
    midiIn = registerMidiPort("midi in", JackPortIsInput);
    return (ringBuffer && audioIn && midiIn);
}

bool Record2MemoryClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    jack_default_audio_sample_t *audioBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(audioIn, nframes);
    void *midiInputBuffer = jack_port_get_buffer(midiIn, nframes);
    jack_nframes_t recordingStart = 0;
    // test if all frames are zero (then recording will stop):
    if (isRecordingProcess) {
        bool stopRecording = true;
        for (jack_nframes_t i = 0; stopRecording && (i < nframes); i++) {
            stopRecording = (audioBuffer[i] == 0);
        }
        if (stopRecording) {
            // tell the QThread that recording has stopped:
            jack_nframes_t framesToWrite = 0;
            if (jack_ringbuffer_write_space(ringBuffer) >= sizeof(jack_nframes_t)) {
                jack_ringbuffer_write(ringBuffer, (const char*)&framesToWrite, sizeof(jack_nframes_t));
                isRecordingProcess = false;
                // tell the QThread that recording has finished:
                waitForAudio.wakeOne();
            }
        }
    } else {
        // read from midi input port and wait for the record signal (note on):
        jack_nframes_t midiInCount = jack_midi_get_event_count(midiInputBuffer);
        for (jack_nframes_t midiInIndex = 0; !isRecordingProcess && (midiInIndex < midiInCount); midiInIndex++) {
            jack_midi_event_t midiEvent;
            jack_midi_event_get(&midiEvent, midiInputBuffer, midiInIndex);
            // look explicitly for note on messages:
            if (midiEvent.size == 3) {
                if ((midiEvent.buffer[0] >> 4) == 0x09) {
                    isRecordingProcess = true;
                    recordingStart = midiEvent.time;
                }
            }
        }

    }
    // so, are we recording?
    if (isRecordingProcess) {
        // record starting with frame at recordingStart,
        // i.e., put all frames starting with that into the ring buffer:
        jack_nframes_t framesToWrite = nframes - recordingStart;
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
    // mutex has to be locked to be used for the wait condition:
    mutexForAudio.lock();
    for (; true; ) {
        // wait for audio to record:
        waitForAudio.wait(&mutexForAudio);
        // read how many frames there are in the ring buffer:
        for (; jack_ringbuffer_read_space(ringBuffer) >= sizeof(jack_nframes_t); ) {
            jack_nframes_t framesToRead = 0;
            jack_ringbuffer_peek(ringBuffer, (char*)&framesToRead, sizeof(jack_nframes_t));
            if (framesToRead == 0) {
                jack_ringbuffer_read_advance(ringBuffer, sizeof(jack_nframes_t));
                // recording has stopped:
                if (audioModelRun) {
                    // push the model into the application thread:
                    audioModelRun->moveToThread(qApp->thread());
                    // lock the mutex first:
                    audioModelsMutex.lock();
                    // put the model in the vector:
                    audioModels.append(audioModelRun);
                    // free the mutex:
                    audioModelsMutex.unlock();
                    audioModelRun = 0;
                    // invoke the corresponding signal:
                    recordingFinished();
                }
            } else if (jack_ringbuffer_read_space(ringBuffer) >= sizeof(jack_nframes_t) + framesToRead * sizeof(jack_default_audio_sample_t)) {
                if (!audioModelRun) {
                    recordingStarted();
                    // create the audio model:
                    audioModelRun = new JackAudioModel();
                    // create a column in the audio model:
                    audioModelRun->insertColumn(0);
                }
                jack_ringbuffer_read_advance(ringBuffer, sizeof(jack_nframes_t));
                // read audio data from the ring buffer and put it into our audio model:
                audioModelRun->appendRowsFromRingBuffer(framesToRead, ringBuffer);
            }
        }
    }
}
