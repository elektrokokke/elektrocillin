#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <vector>
#include <iostream>
#include <QString>
#include <complex>

template<class T> class Polynomial : public std::vector<T>
{
public:
    /**
      Constructs a polynomial of degree 0 and initializes
      it with 0.
      */
    Polynomial() :
        std::vector<T>(1, 0)
    {}
    /**
      Constructs a polynomial of degree 0 and initializes
      it with the given constant.
      */
    Polynomial(const T &constant) :
        std::vector<T>(1, constant)
    {}
    /**
      Constructs a polynomial of degree 1 and initializes
      iit with the given constant and first degree coefficient.
      */
    Polynomial(const T &constant, const T &firstDegree) :
        std::vector<T>(2)
    {
        std::vector<T>::at(0) = constant;
        std::vector<T>::at(1) = firstDegree;
    }
    /**
      Constructs a copy of the given polynomial.
      */
    Polynomial(const std::vector<T> &tocopy) :
        std::vector<T>(tocopy)
    {}

    /**
      Returns the degree of this polynomial.
      */
    size_t degree() const
    {
        return std::vector<T>::size() - 1;
    }

    /**
      Evaluate the polyniomial at the given x and return
      the result.
      */
    T evaluate(const T &at)
    {
        T result = this->at(0);
        T x = at;
        for (size_t i = 1; i < this->size(); i++) {
            result += this->at(i) * x;
            x *= at;
        }
        return result;
    }

    /**
      Add the given polynomial to this polynomial.
      */
    Polynomial<T>& operator+=(const Polynomial<T> &toadd)
    {
        if (std::vector<T>::size() < toadd.size()) {
            size_t i = 0;
            for (; i < std::vector<T>::size(); i++) {
                std::vector<T>::at(i) += toadd.at(i);
            }
            resize(toadd.size());
            for (; i < std::vector<T>::size(); i++) {
                std::vector<T>::at(i) = toadd.at(i);
            }
        } else {
            for (size_t i = 0; i < toadd.size(); i++) {
                std::vector<T>::at(i) += toadd.at(i);
            }
        }
        return *this;
    }

    /**
      Multiply this polynomial with the given constant.
      */
    Polynomial<T>& operator*=(const T &factor)
    {
        for (size_t i = 0; i < std::vector<T>::size(); i++) {
            std::vector<T>::at(i) *= factor;
        }
        return *this;
    }

    /**
      Multiply this polynomial with the given polynomial.
      */
    Polynomial<T>& operator*=(const Polynomial<T> &tomultiply)
    {
        std::vector<T> product(degree() + tomultiply.degree() + 1);
        for (size_t i = std::vector<T>::size(); i != 0; ) {
            i--;
            for (size_t j = tomultiply.size(); j != 0; ) {
                j--;
                product.at(i + j) += std::vector<T>::at(i) * tomultiply.at(j);
            }
        }
        resize(product.size());
        for (size_t i = 0; i < std::vector<T>::size(); i++) {
            std::vector<T>::at(i) = product.at(i);
        }
        return *this;
    }

    /**
      Assign this polynomial to the integral of the given polynomial.
      */
    void integrate(const Polynomial<T> &tointegrate)
    {
        resize(tointegrate.size() + 1);
        std::vector<T>::at(0) = 0;
        for (size_t i = 0; i < std::vector<T>::size(); i++) {
            std::vector<T>::at(i + 1) = tointegrate.at(i) / (T)(i + 1);
        }
    }

    /**
      Adjust the constant part such that the polynomial at
      the given x equals the given y.
      */
    void adjustConstantToIntersect(const T &x, const T &y)
    {
        T value = evaluate(x);
        std::vector<T>::at(0) += y - value;
    }
};

/**
  Creates and returns a string representation of this polynomial.
  Only works for complex numbers currently.
  */
template<class T> QString polynomialToString(const Polynomial<std::complex<T> > &polynomial)
{
    QString string;
    for (size_t i = 0; i < polynomial.size(); i++) {
        string += QString("(%1,%2i)*x^%3").arg(polynomial.at(i).real()).arg(polynomial.at(i).imag()).arg(i);
        if (i < polynomial.size() - 1) {
            string += "+";
        }
    }
    return string;
}


#endif // POLYNOMIAL_H
