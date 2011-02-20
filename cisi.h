#ifndef CISI_H
#define CISI_H

#include <complex>

class Cisi {
public:
    static double si(const double x);
    static void cisi(const double x, std::complex<double> &cs);
};


#endif // CISI_H
