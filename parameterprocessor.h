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

#include <QMap>
#include <QString>
#include <QVector>

class ParameterProcessor
{
public:
    struct Parameter {
        Parameter() {}
        Parameter(const QString &name_, double value_, double min_, double max_, double resolution_, QMap<double, QString> stringValues_) :
            name(name_), value(value_), min(min_), max(max_), resolution(resolution_), stringValues(stringValues_)
        {}
        QString name;
        double value, min, max, resolution;
        QMap<double, QString> stringValues;
    };

    ParameterProcessor();
    ParameterProcessor(const ParameterProcessor &tocopy);

    ParameterProcessor & operator=(const ParameterProcessor &parameterProcessor);

    virtual int registerParameter(const QString &name, double value, double min, double max, double resolution, QMap<double, QString> stringValues = QMap<double, QString>());

    virtual int getNrOfParameters() const;
    virtual Parameter & getParameter(int index);
    virtual const Parameter & getParameter(int index) const;
    /**
      Changes the value, minimum and maximum of a parameter with given
      index.

      @return true, if value, min or max of the parameter with given index
        differ from their previous values, false otherwise
      */
    virtual bool setParameterValue(int index, double value, double min, double max, unsigned int time);
    /**
      Convenience method that calls the above method with the current min and max values.
      */
    bool setParameterValue(int index, double value, unsigned int time);

    bool hasParameterChanged(int index) const;
    bool hasAnyParameterChanged() const;
    void resetParameterChanged();

private:
    QVector<Parameter> parameters;
    QVector<bool> parametersChanged;
    bool anyParameterChanged;
};

#endif // PARAMETERPROCESSOR_H
