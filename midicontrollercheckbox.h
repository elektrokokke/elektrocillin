#ifndef MIDICONTROLLERCHECKBOX_H
#define MIDICONTROLLERCHECKBOX_H

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

#include <QCheckBox>

class MidiControllerCheckBox : public QCheckBox
{
    Q_OBJECT
    Q_PROPERTY(unsigned char channel READ getChannel WRITE setChannel)
    Q_PROPERTY(unsigned char controller READ getController WRITE setController)
public:
    explicit MidiControllerCheckBox(QWidget *parent = 0);

    unsigned char getChannel() const;
    unsigned char getController() const;

signals:
    void controlChanged(unsigned char channel, unsigned char controller, unsigned char value);

public slots:
    void setChannel(unsigned char channel);
    void setController(unsigned char controller);
    void onControlChange(unsigned char channel, unsigned char controller, unsigned char value);

private slots:
    void onToggled(bool checked);

private:
    char channel, controller;
    bool changingValue;
};

#endif // MIDICONTROLLERCHECKBOX_H
