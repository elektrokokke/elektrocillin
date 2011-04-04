#ifndef PARAMETERPROCESSOR_H
#define PARAMETERPROCESSOR_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

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

#include <QString>
#include <QVector>

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

    ParameterProcessor();
    ParameterProcessor(const ParameterProcessor &tocopy);

    ParameterProcessor & operator=(const ParameterProcessor &parameterProcessor);

    virtual int registerParameter(const QString &name, double value, double min, double max, double resolution);

    virtual int getNrOfParameters() const;
    virtual Parameter & getParameter(int index);
    virtual const Parameter & getParameter(int index) const;
    /**
      @return true, if the previous value of the parameter with given index
        differed from the given value, false otherwise
      */
    virtual bool setParameterValue(int index, double value);
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
    virtual bool hasParameterChanged(int index);

private:
    QVector<Parameter> parameters;
    QVector<bool> parametersChanged;
};

#endif // PARAMETERPROCESSOR_H
