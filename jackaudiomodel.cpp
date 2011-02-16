#include "jackaudiomodel.h"
#include "metajack/jack.h"

JackAudioModel::JackAudioModel(QObject *parent) :
    FloatTableModel(parent)
{
}

void JackAudioModel::appendRowsFromRingBuffer ( int count, jack_ringbuffer_t *ringBuffer )
{
    // determine how many audio frames are in the ring buffer:
    int oldSize = rowCount();
    int newSize = oldSize + count;
    // notify that frames (here they correspond to rows) are being added:
    beginInsertRows(QModelIndex(), oldSize, newSize - 1);
    // resize the underlying float vectors:
    for (int col = 0; col < columnCount(); col++) {
        audioBuffers[col]->resize(newSize);
    }
    // read from the ring buffer and set the vector entries accordingly:
    QVector<jack_default_audio_sample_t> frame(columnCount());
    for (int col = 0; col < columnCount(); col++) {
        // read one frame (i.e., columnCount() samples):
        jack_ringbuffer_read(ringBuffer, (char*)(audioBuffers[col]->data() + oldSize), count * sizeof(jack_default_audio_sample_t));
    }
    // notify that frames (here they correspond to rows) have been added:
    endInsertRows();
}
