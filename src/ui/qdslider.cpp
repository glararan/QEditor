/*This file is part of QEditor.

QEditor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QEditor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QEditor.  If not, see <http://www.gnu.org/licenses/>.*/

#include "qdslider.h"

#include "helper/mathhelper.h"

QDSlider::QDSlider(int decimals, QWidget* parent) : QSlider(parent)
{
    setOrientation(Qt::Horizontal);

    QString fDecimal("1");

    for(int i = 0; i < decimals; i++)
        fDecimal += "0";

    multiplier = fDecimal.toDouble();
    decs       = decimals;

    connect(this, SIGNAL(valueChanged(int)), this, SLOT(intToDouble(int)));
}

void QDSlider::intToDouble(int value)
{
    double val = MathHelper::toDouble(value);
    val /= multiplier;

    emit valueChanged(val);
}

void QDSlider::setMinimum(double value)
{
    value = QString::number(value, 'f', decs).toDouble() * multiplier;

    QSlider::setMinimum(value);
}

void QDSlider::setMaximum(double value)
{
    value = QString::number(value, 'f', decs).toDouble() * multiplier;

    QSlider::setMaximum(value);
}

void QDSlider::setValue(double value)
{
    value = QString::number(value, 'f', decs).toDouble() * multiplier;

    QSlider::setValue(value);
}

double QDSlider::value() const
{
    return QSlider::value() / multiplier;
}