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

#ifndef QDSLIDER_H
#define QDSLIDER_H

#include <QSlider>
#include <QtWidgets/QWidget>

class QDSlider : public QSlider
{
    Q_OBJECT

public:
    QDSlider(int decimals = 2, QWidget* parent = 0);

    void setMinimum(double value);
    void setMaximum(double value);
    void setValue(double value);

    double value() const;

private:
    double multiplier;

    int decs;

signals:
    void valueChanged(double value);

private slots:
    void intToDouble(int value);
};

#endif // QDSLIDER_H