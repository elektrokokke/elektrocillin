#ifndef PARAMETERPROCESSOR_H
#define PARAMETERPROCESSOR_H

#include <QString>

class ParameterProcessor
{
public:
    struct Parameter {
        Parameter() {}
        Parameter(const QString &name_, double value_, double min_, double max_, double resolution_) :
            name(name_), value(value_), min(min_), max(max_), resolution(resolution_)
        {}
        QString name;
        double value, min, max, resolution;
    };
    virtual int getNrOfParameters() const = 0;
    virtual const Parameter & getParameter(int index) const = 0;
    virtual void setParameterValue(int index, double value) = 0;
    /**
      This function should return wether the value of the parameter with the given
      index has been changed since the last call to this function with the same
      parameter.

      If the functions has not been called yet since object construction,
      this function should return true if and only if the parameter value
      has been changed at all.

      After call to this method another call with the same parameter should
      return false unless the value was changed again between calls.
      */
    virtual bool hasParameterChanged(int index) = 0;
};

#endif // PARAMETERPROCESSOR_H
