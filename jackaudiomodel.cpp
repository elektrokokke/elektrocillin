/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jackaudiomodel.h"
#include "metajack/metajack.h"

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
