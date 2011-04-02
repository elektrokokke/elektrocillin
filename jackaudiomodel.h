#ifndef JACKAUDIOMODEL_H
#define JACKAUDIOMODEL_H

/*
    Copyright 2011 Arne Jacobs

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "floattablemodel.h"
#include <jack/ringbuffer.h>

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
