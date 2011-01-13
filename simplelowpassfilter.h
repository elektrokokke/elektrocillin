#ifndef SIMPLELOWPASSFILTER_H
#define SIMPLELOWPASSFILTER_H

class SimpleLowpassFilter
{
public:
    SimpleLowpassFilter(double a = 0.1, double b = 0.9);

    double filter(double sample);

private:
    double previous, a, b;
};

#endif // SIMPLELOWPASSFILTER_H
