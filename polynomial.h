#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <vector>

template<class T> class Polynomial : public std::vector<T>
{
public:
    Polynomial(const T &constant) :
        std::vector<T>(1, constant)
    {}
    Polynomial(const T &constant, const T &firstDegree) :
        std::vector<T>(2)
    {
        std::vector<T>::at(0) = constant;
        std::vector<T>::at(1) = firstDegree;
    }

    Polynomial(const Polynomial<T> &tocopy) :
        std::vector<T>(tocopy)
    {}

    size_t degree() const
    {
        return std::vector<T>::size() - 1;
    }

    // add another polynomial:
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

    // multiply with a constant:
    Polynomial<T>& operator*=(const T &factor)
    {
        for (size_t i = 0; i < std::vector<T>::size(); i++) {
            std::vector<T>::at(i) *= factor;
        }
        return *this;
    }

    // multiply with another polynomial:
    Polynomial<T>& operator*=(const Polynomial<T> &tomultiply)
    {
        size_t oldSize = std::vector<T>::size();
        resize(degree() + tomultiply.degree() + 1);
        for (size_t i = oldSize; i != 0; ) {
            i--;
            for (size_t j = tomultiply.size(); j != 0; ) {
                j--;
                std::vector<T>::at(i + j) = std::vector<T>::at(i) * tomultiply.at(j);
            }
        }
        return *this;
    }
};

#endif // POLYNOMIAL_H
