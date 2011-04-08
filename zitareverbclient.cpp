/*
    Copyright 2010-2011 Fons Adriaensen <fons@kokkinizita.net>,
                        Arne Jacobs <jarne@jarne.de>

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

#include "zitareverbclient.h"

QStringList ZitaReverbClient::audioInputPortNames = QStringList("in.L") + QStringList("in.R");
QStringList ZitaReverbClient::audioOutputPortNames = QStringList("out.L") + QStringList("out.R");
QStringList ZitaReverbClient::outputPortNamesAmbis = QStringList("out.W") + QStringList("out.X") + QStringList("out.X") + QStringList("out.Z");

ZitaReverbClient::ZitaReverbClient(const QString &clientName, bool ambis) :
    AudioProcessorClient(clientName, audioInputPortNames, (ambis ? outputPortNamesAmbis : audioOutputPortNames)),
    _ambis(ambis)
{
}

bool ZitaReverbClient::init()
{
    if (AudioProcessorClient::init()) {
        // initialize the reverb:
        _fragm = 1024;
        _nsamp = 0;
        _reverb.init (getSampleRate(), _ambis);
        return true;
    } else {
        return false;
    }
}

bool ZitaReverbClient::process(jack_nframes_t frames)
{
    getAudioPortBuffers(frames);

    int   i, k, n_inp, n_out;
    float *inp [2];
    float *out [4];

    n_inp = 2;
    n_out = _ambis ? 4 : 2;
    for (i = 0; i < n_inp; i++) inp [i] = (float *) getInputBuffer(i);
    for (i = 0; i < n_out; i++) out [i] = (float *) getOutputBuffer(i);

    while (frames)
    {
        if (!_nsamp)
        {
            _reverb.prepare (_fragm);
            _nsamp = _fragm;
        }
        k = (_nsamp < frames) ? _nsamp : frames;
        _reverb.process (k, inp, out);
        for (i = 0; i < n_inp; i++) inp [i] += k;
        for (i = 0; i < n_out; i++) out [i] += k;
        frames -= k;
        _nsamp -= k;
    }

    return true;
}

class ZitaReverbClientFactory : public JackClientFactory
{
public:
    ZitaReverbClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Effect (Zita-rev1)";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new ZitaReverbClient(clientName);
    }
    static ZitaReverbClientFactory factory;
};

ZitaReverbClientFactory ZitaReverbClientFactory::factory;

JackClientFactory * ZitaReverbClient::getFactory()
{
    return &ZitaReverbClientFactory::factory;
}
