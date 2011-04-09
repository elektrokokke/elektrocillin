#ifndef ZITAREVERBCLIENT_H
#define ZITAREVERBCLIENT_H

/*
    Copyright 2010-2011 Fons Adriaensen <fons@kokkinizita.net>,
                        Arne Jacobs <jarne@jarne.de>

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

#include "audioprocessorclient.h"
#include "reverb.h"

class ZitaReverbClient : public AudioProcessorClient
{
public:
    ZitaReverbClient(const QString &clientName, bool ambis = false);

    // reimplemented from JackClient:
    JackClientFactory * getFactory();
protected:
    // reimplemented from JackClient:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);
private:
    unsigned int _fragm;
    unsigned int _nsamp;
    bool _ambis;
    Reverb _reverb;

    static QStringList audioInputPortNames, audioOutputPortNames, outputPortNamesAmbis;
};

#endif // ZITAREVERBCLIENT_H
