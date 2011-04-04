#ifndef MIDIPITCHSLIDER_H
#define MIDIPITCHSLIDER_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

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

#include <QSlider>

class MidiPitchSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(unsigned char channel READ getChannel WRITE setChannel)
public:
    explicit MidiPitchSlider(QWidget *parent = 0);

    unsigned char getChannel() const;

signals:
    void pitchWheel(unsigned char channel, unsigned int pitch);

public slots:
    void setChannel(unsigned char channel);
    void onPitchWheel(unsigned char channel, unsigned int pitch);

protected:
    virtual void mouseReleaseEvent ( QMouseEvent * ev );

private slots:
    void onValueChanged(int value);

private:
    char channel;
    bool changingValue;
};

#endif // MIDIPITCHSLIDER_H
