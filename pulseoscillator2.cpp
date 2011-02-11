#include "pulseoscillator2.h"
#include <cmath>

PulseOscillator2::PulseOscillator2(double frequencyModulationIntensity, double sampleRate) :
    Oscillator(frequencyModulationIntensity, sampleRate, QStringList("pulsewidth_modulation_in")),
    pulseIntegral(QVector<double>(3), QVector<double>(3))
{
    // initialize the pulse integral control points:
    pulseIntegral.getX()[0] = 0;
    pulseIntegral.getY()[0] = 0;
    pulseIntegral.getX()[1] = M_PI;
    pulseIntegral.getY()[1] = M_PI;
    pulseIntegral.getX()[2] = 2 * M_PI;
    pulseIntegral.getY()[2] = 0;
}

void PulseOscillator2::setPulseWidth(double pulseWidth)
{
    if (pulseWidth < 0) {
        pulseWidth = 0;
    } else if (pulseWidth > 2 * M_PI) {
        pulseWidth = 2 * M_PI;
    }
    pulseIntegral.getX()[1] = pulseWidth;
    pulseIntegral.getY()[1] = pulseWidth;
    pulseIntegral.getY()[2] = (pulseWidth - M_PI) * 2;
}

void PulseOscillator2::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    // process the second input (to modulate pulse width):
    setPulseWidth(inputs[1] * M_PI * 0.9 + M_PI);
    // create the sound:
    Oscillator::processAudio(inputs, outputs, time);
}

double PulseOscillator2::valueAtPhase(double phase, double previousPhase)
{
    if (phase == previousPhase) {
        return 0;
    }
    double previousIntegralValue = pulseIntegral.evaluate(previousPhase);
    // compare current phase with previous phase:
    if (phase < previousPhase) {
        // phase has wrapped around, adjust the previous integral level accordingly:
        previousIntegralValue -= pulseIntegral.getY()[2];
        previousPhase -= 2 * M_PI;
    }
    double integralValue = pulseIntegral.evaluate(phase);
    // integral difference / phase difference is the oscillator output:
    double output = (integralValue - previousIntegralValue) / (phase - previousPhase);
    return output;
}
