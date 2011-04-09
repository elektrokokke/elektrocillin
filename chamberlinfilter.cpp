#include "chamberlinfilter.h"
#include <cmath>

ChamberlinFilter::ChamberlinFilter() :
    AudioProcessor(QStringList("Audio in"), QStringList("Low") + QStringList("High") + QStringList("Band") + QStringList("Notch")),
    MidiParameterProcessor(QStringList(), QStringList()),
    low(0),
    band(0)
{
    // cutoff frequency in Hertz (default is a quarter the sample rate)
    registerParameter("Base cutoff frequency", 440, 0, 0, 0);
    // resonance [0:1] (default is zero)
    registerParameter("Resonance", 0, 0, 1, 0);
}

void ChamberlinFilter::setSampleRate(double sampleRate)
{
    AudioProcessor::setSampleRate(sampleRate);
    // adapt the maximum cutoff frequency:
    getParameter(1).max = 0.2 * sampleRate;
}

void ChamberlinFilter::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    double cutoff = getParameter(1).value;
    double q = 1.0 - getParameter(2).value;
    // compute coefficients:
    f = 2.0 * sin(M_PI * cutoff * getSampleDuration());
    scale = 1.0;//sqrt(q);
    // compute output:
    low = low + f * band;
    high = scale * inputs[0] - low - q * band;
    band = f * high + band;
    notch = high + low;
    outputs[0] = low;
    outputs[1] = high;
    outputs[2] = band;
    outputs[3] = notch;
}

ChamberlinFilterClient::ChamberlinFilterClient(const QString &clientName, ChamberlinFilter *processFilter_, ChamberlinFilter *guiFilter_, size_t ringBufferSize) :
    ParameterClient(clientName, processFilter_, processFilter_, 0, processFilter_, guiFilter_, ringBufferSize),
    processFilter(processFilter_),
    guiFilter(guiFilter_)
{
}

ChamberlinFilterClient::~ChamberlinFilterClient()
{
    // calling close will stop the Jack client:
    close();
    // deleting the filter is now safe, as it is not used anymore (the Jack process thread is stopped):
    delete processFilter;
    delete guiFilter;
}

bool ChamberlinFilterClient::init()
{
    if (ParameterClient::init()) {
        // adjust the guiFilter's samplerate to that of the processFilter:
        guiFilter->setSampleRate(processFilter->getSampleRate());
        return true;
    } else {
        return false;
    }
}

class ChamberlinFilterClientFactory : public JackClientFactory
{
public:
    ChamberlinFilterClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Filter (Chamberlin)";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new ChamberlinFilterClient(clientName, new ChamberlinFilter(), new ChamberlinFilter());
    }
    static ChamberlinFilterClientFactory factory;
};

ChamberlinFilterClientFactory ChamberlinFilterClientFactory::factory;

JackClientFactory * ChamberlinFilterClient::getFactory()
{
    return &ChamberlinFilterClientFactory::factory;
}
