#include <cmath>
#include <complex>
#include <limits>
#include <QDebug>
using namespace std;

void cisi(const double x, complex<double> &cs)
{
        const int MAXIT=100;
        const double EULER=0.577215664901533, PIBY2=1.570796326794897, TMIN=2.0;
        const double EPS=numeric_limits<double>::epsilon();
        const double FPMIN=numeric_limits<double>::min()*4.0;
        const double BIG=numeric_limits<double>::max()*EPS;
        int i,k;
        bool odd;
        double a,err,fact,sign,sum,sumc,sums,t,term;
        complex<double> h,b,c,d,del;

        t=fabs(x);
        if (t == 0.0) {
                cs= -BIG;
                return;
        }
        if (t > TMIN) {
                b=complex<double>(1.0,t);
                c=complex<double>(BIG,0.0);
                d=h=1.0/b;
                for (i=1;i<MAXIT;i++) {
                        a= -i*i;
                        b += 2.0;
                        d=1.0/(a*d+b);
                        c=b+a/c;
                        del=c*d;
                        h *= del;
                        if (fabs(real(del)-1.0)+fabs(imag(del)) <= EPS) break;
                }
                Q_ASSERT_X(!(i >= MAXIT), "cisi.cpp", "cf failed in cisi");
                h=complex<double>(cos(t),-sin(t))*h;
                cs= -conj(h)+complex<double>(0.0,PIBY2);
        } else {
                if (t < sqrt(FPMIN)) {
                        sumc=0.0;
                        sums=t;
                } else {
                        sum=sums=sumc=0.0;
                        sign=fact=1.0;
                        odd=true;
                        for (k=1;k<=MAXIT;k++) {
                                fact *= t/k;
                                term=fact/k;
                                sum += sign*term;
                                err=term/fabs(sum);
                                if (odd) {
                                        sign = -sign;
                                        sums=sum;
                                        sum=sumc;
                                } else {
                                        sumc=sum;
                                        sum=sums;
                                }
                                if (err < EPS) break;
                                odd=!odd;
                        }
                        Q_ASSERT_X(!(k > MAXIT), "cisi.cpp", "maxits exceeded in cisi");
                }
                cs=complex<double>(sumc+log(t)+EULER,sums);
        }
        if (x < 0.0) cs = conj(cs);
}
