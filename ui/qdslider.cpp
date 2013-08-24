#include "qdslider.h"

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
    double val = static_cast<double>(value);
    val /= multiplier;

    emit valueChanged(val);
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
