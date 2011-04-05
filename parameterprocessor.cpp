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

#include "parameterprocessor.h"

ParameterProcessor::ParameterProcessor()
{
}

ParameterProcessor::ParameterProcessor(const ParameterProcessor &tocopy) :
    parameters(tocopy.parameters),
    parametersChanged(tocopy.parametersChanged)
{
}

ParameterProcessor & ParameterProcessor::operator=(const ParameterProcessor &parameterProcessor)
{
    parameters = parameterProcessor.parameters;
    parametersChanged = parameterProcessor.parametersChanged;
    return *this;
}

int ParameterProcessor::registerParameter(const QString &name, double value, double min, double max, double resolution)
{
    int id = parameters.size();
    parameters.append(Parameter(name, value, min, max, resolution));
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

bool ParameterProcessor::setParameterValue(int index, double value, unsigned int time)
{
    Q_ASSERT(index < parameters.size());
    Q_ASSERT(index < parametersChanged.size());
    if (parameters[index].value != value) {
        parameters[index].value = value;
        parametersChanged[index] = true;
        return true;
    } else {
        return false;
    }
}

bool ParameterProcessor::hasParameterChanged(int index)
{
    Q_ASSERT(index < parametersChanged.size());
    if (parametersChanged[index]) {
        parametersChanged[index] = false;
        return true;
    } else {
        return false;
    }
}
