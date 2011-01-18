#ifndef JACKAUDIOMODEL_H
#define JACKAUDIOMODEL_H

#include "floattablemodel.h"
#include "jack/ringbuffer.h"

class JackAudioModel : public FloatTableModel
{
    Q_OBJECT
public:
    explicit JackAudioModel(QObject *parent = 0);

    void appendRowsFromRingBuffer ( int count, jack_ringbuffer_t *ringBuffer );

signals:

public slots:

};

#endif // JACKAUDIOMODEL_H
