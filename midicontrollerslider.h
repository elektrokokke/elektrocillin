#ifndef MIDICONTROLLERSLIDER_H
#define MIDICONTROLLERSLIDER_H

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

#include <QSlider>

class MidiControllerSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(unsigned char channel READ getChannel WRITE setChannel)
    Q_PROPERTY(unsigned char controller READ getController WRITE setController)
public:
    explicit MidiControllerSlider(QWidget *parent = 0);

    unsigned char getChannel() const;
    unsigned char getController() const;

signals:
    void controlChanged(unsigned char channel, unsigned char controller, unsigned char value);

public slots:
    void setChannel(unsigned char channel);
    void setController(unsigned char controller);
    void onControlChange(unsigned char channel, unsigned char controller, unsigned char value);

private slots:
    void onValueChanged(int value);

private:
    char channel, controller;
    bool changingValue;

};

#endif // MIDICONTROLLERSLIDER_H
