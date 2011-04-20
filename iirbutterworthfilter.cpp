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

#include "iirbutterworthfilter.h"
#include "frequencyresponsegraphicsitem.h"
#include <cmath>

IirButterworthFilter::IirButterworthFilter() :
    AudioProcessor(QStringList("Audio in"), QStringList("Low") + QStringList("High") + QStringList("Band")),
    MidiParameterProcessor(QStringList("Midi note in"), QStringList()),
    lowpass(440, IirButterworthFilter2::LOW_PASS),
    highpass(440, IirButterworthFilter2::HIGH_PASS),
    bandpass(lowpass)
{
    // cutoff frequency in Hertz (default is a quarter the sample rate)
    registerParameter("Base cutoff frequency", 440, 0, 0, 0);
    // offset between Midi note frequency and resulting cutoff frequency in semitones (default is zero)
    registerParameter("Midi note frequency offset", 0, -36, 36, 1);
    // uneditable parameters for cutoff modulation from audio, pitch bend and controller, and for resonance modulation from audio and controller:
    registerParameter("Pitch bend", 0, 0, 0, 0);

}

void IirButterworthFilter::setSampleRate(double sampleRate)
{
    AudioProcessor::setSampleRate(sampleRate);
    lowpass.setSampleRate(sampleRate);
    highpass.setSampleRate(sampleRate);
    bandpass.setSampleRate(sampleRate);
    // adapt the maximum cutoff frequency:
    getParameter(1).max = 0.5 * sampleRate;
    computeCoefficients();
}

void IirButterworthFilter::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    lowpass.processAudio(inputs, outputs, time);
    highpass.processAudio(inputs, outputs + 1, time);
    bandpass.processAudio(inputs, outputs + 2, time);
}

void IirButterworthFilter::processNoteOn(int inputIndex, unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t time)
{
    if (inputIndex == 1) {
        // set base cutoff frequency from note number:
        ParameterProcessor::setParameterValue(1, computeFrequencyFromMidiNoteNumber(noteNumber) * pow(2.0, getParameter(2).value / 12.0), time);
    }
}

void IirButterworthFilter::processPitchBend(int inputIndex, unsigned char, unsigned int value, jack_nframes_t time)
{
    if (inputIndex == 1) {
        // cutoff modulation through pitch bend wheel:
        int pitchCentered = (int)value - 0x2000;
        ParameterProcessor::setParameterValue(3, (double)pitchCentered / 8192.0, time);
    }
}

bool IirButterworthFilter::setParameterValue(int index, double value, double min, double max, unsigned int time)
{
    if (MidiParameterProcessor::setParameterValue(index, value, min, max, time)) {
        computeCoefficients();
        return true;
    } else {
        return false;
    }
}

FrequencyResponse * IirButterworthFilter::getLowpassResponse()
{
    return &lowpass;
}

FrequencyResponse * IirButterworthFilter::getHighpassResponse()
{
    return &highpass;
}

FrequencyResponse * IirButterworthFilter::getBandpassResponse()
{
    return &bandpass;
}

void IirButterworthFilter::computeCoefficients()
{
    double cutoffFrequencyInHertz = getParameter(1).value * pow(2.0, (getParameter(2).value * getParameter(3).value) / 12.0);
    lowpass.setCutoffFrequency(cutoffFrequencyInHertz);
    highpass.setCutoffFrequency(cutoffFrequencyInHertz);
    bandpass.copyCoefficients(lowpass);
    bandpass *= highpass;
}

IirButterworthFilter2::IirButterworthFilter2(double cutoffFrequencyInHertz, Type type_) :
    IirFilter(3, 2, QStringList()),
    type(type_)
{
    setCutoffFrequency(cutoffFrequencyInHertz);
}

void IirButterworthFilter2::setCutoffFrequency(double cutoffFrequencyInHertz, Type type)
{
    this->cutoffFrequency = cutoffFrequencyInHertz;
    this->type = type;
    double radians = convertHertzToRadians(type == LOW_PASS ? cutoffFrequency : getSampleRate() * 0.5 - cutoffFrequencyInHertz);
    if (radians <= 0) {
        radians = 0.000000000001;
    } else if (radians >= M_PI) {
        radians = M_PI - 0.000000000001;
    }
    double c = cos(radians * 0.5) / sin(radians * 0.5);
    double factor = 1 / (c*c + c*sqrt(2.0) + 1.0);
    getFeedBackCoefficients()[0] = (-c*c*2.0 + 2.0) * factor;
    getFeedBackCoefficients()[1] = (c*c - c*sqrt(2.0) + 1.0) * factor;
    getFeedForwardCoefficients()[0] = factor;
    getFeedForwardCoefficients()[1] = 2 * factor;
    getFeedForwardCoefficients()[2] = factor;
    if (type == HIGH_PASS) {
        invert();
    }
}

void IirButterworthFilter2::setCutoffFrequency(double cutoffFrequencyInHertz)
{
    setCutoffFrequency(cutoffFrequencyInHertz, getType());
}

void IirButterworthFilter2::setType(Type type)
{
    setCutoffFrequency(getCutoffFrequency(), type);
}

double IirButterworthFilter2::getCutoffFrequency() const
{
    return cutoffFrequency;
}

IirButterworthFilter2::Type IirButterworthFilter2::getType() const
{
    return type;
}

IirButterworthFilterClient::IirButterworthFilterClient(const QString &clientName, IirButterworthFilter *processFilter_, IirButterworthFilter *guiFilter_, size_t ringBufferSize) :
    ParameterClient(clientName, processFilter_, processFilter_, 0, processFilter_, guiFilter_, ringBufferSize),
    processFilter(processFilter_),
    guiFilter(guiFilter_)
{
}

IirButterworthFilterClient::~IirButterworthFilterClient()
{
    // calling close will stop the Jack client:
    close();
    // deleting the filter is now safe, as it is not used anymore (the Jack process thread is stopped):
    delete processFilter;
    delete guiFilter;
}

QGraphicsItem * IirButterworthFilterClient::createGraphicsItem()
{
    int padding = 4;
    QGraphicsRectItem *item = new QGraphicsRectItem();
    QGraphicsItem *parameterItem = ParameterClient::createGraphicsItem();
    QRectF rect = QRect(0, 0, 600, 420);
    rect = rect.translated(parameterItem->boundingRect().width() + 2 * padding, padding);
    parameterItem->setPos(padding, padding);
    parameterItem->setParentItem(item);

    FrequencyResponseGraphicsItem *ourItem = new FrequencyResponseGraphicsItem(rect, 22050.0 / 512.0, 22050, -50, 10, item);
    ourItem->addFrequencyResponse(guiFilter->getLowpassResponse());
    ourItem->addFrequencyResponse(guiFilter->getHighpassResponse());
    ourItem->addFrequencyResponse(guiFilter->getBandpassResponse());
    QObject::connect(this, SIGNAL(changedParameterValue(int,double,double,double)), ourItem, SLOT(updateFrequencyResponses()));

    item->setRect((rect | parameterItem->boundingRect().translated(parameterItem->pos())).adjusted(-padding, -padding, padding, padding));
    item->setPen(QPen(QBrush(Qt::black), 1));
    item->setBrush(QBrush(Qt::white));
    return item;
}

bool IirButterworthFilterClient::init()
{
    if (ParameterClient::init()) {
        // adjust the guiFilter's samplerate to that of the processFilter:
        guiFilter->setSampleRate(processFilter->getSampleRate());
        return true;
    } else {
        return false;
    }
}

class IirButterworthFilterClientFactory : public JackClientFactory
{
public:
    IirButterworthFilterClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Filter (Butterworth)";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new IirButterworthFilterClient(clientName, new IirButterworthFilter(), new IirButterworthFilter());
    }
    static IirButterworthFilterClientFactory factory;
};

IirButterworthFilterClientFactory IirButterworthFilterClientFactory::factory;

JackClientFactory * IirButterworthFilterClient::getFactory()
{
    return &IirButterworthFilterClientFactory::factory;
}
