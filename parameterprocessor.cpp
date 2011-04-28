/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "parameterprocessor.h"

ParameterProcessor::ParameterProcessor() :
    anyParameterChanged(false)
{
}

ParameterProcessor::ParameterProcessor(const ParameterProcessor &tocopy) :
    parameters(tocopy.parameters),
    parametersChanged(tocopy.parametersChanged),
    anyParameterChanged(tocopy.anyParameterChanged)
{
}

ParameterProcessor & ParameterProcessor::operator=(const ParameterProcessor &parameterProcessor)
{
    parameters = parameterProcessor.parameters;
    parametersChanged = parameterProcessor.parametersChanged;
    anyParameterChanged = parameterProcessor.anyParameterChanged;
    return *this;
}

int ParameterProcessor::registerParameter(const QString &name, double value, double min, double max, double resolution)
{
    registerParameter(name, value, min, max, resolution, QMap<double, QString>());
}

int ParameterProcessor::registerParameter(const QString &name, double value, double min, double max, double resolution, QMap<double, QString> stringValues)
{
    int id = parameters.size();
    parameters.append(Parameter(name, value, min, max, resolution, stringValues));
    parametersChanged.append(false);
    return id;
}

int ParameterProcessor::getNrOfParameters() const
{
    return parameters.size();
}

ParameterProcessor::Parameter & ParameterProcessor::getParameter(int index)
{
    Q_ASSERT(index < parameters.size());
    return parameters[index];
}

const ParameterProcessor::Parameter & ParameterProcessor::getParameter(int index) const
{
    Q_ASSERT(index < parameters.size());
    return parameters[index];
}

bool ParameterProcessor::setParameterValue(int index, double value, double min, double max, unsigned int time)
{
    Q_ASSERT(index < parameters.size());
    Q_ASSERT(index < parametersChanged.size());
    if ((parameters[index].value != value) || (parameters[index].min != min) || (parameters[index].max != max)) {
        parameters[index].value = (min != max ? qBound(min, value, max) : value);
        parameters[index].min = min;
        parameters[index].max = max;
        parametersChanged[index] = true;
        anyParameterChanged = true;
        return true;
    } else {
        return false;
    }
}

bool ParameterProcessor::setParameterValue(int index, double value, unsigned int time)
{
    const Parameter &parameter = getParameter(index);
    return setParameterValue(index, value, parameter.min, parameter.max, time);
}

bool ParameterProcessor::hasParameterChanged(int index) const
{
    Q_ASSERT(index < parametersChanged.size());
    return parametersChanged[index];
}

bool ParameterProcessor::hasAnyParameterChanged() const
{
    return anyParameterChanged;
}

void ParameterProcessor::resetParameterChanged()
{
    for (int i = 0; i < parameters.size(); i++) {
        parametersChanged[i] = false;
    }
    anyParameterChanged = false;
}
