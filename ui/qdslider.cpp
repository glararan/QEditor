#include "qdslider.h"

QDSlider::QDSlider(QWidget* parent) : QSlider(parent)
{
    setOrientation(Qt::Horizontal);

    connect(this, SIGNAL(valueChanged(int)), this, SLOT(intToDouble(int)));
}

void QDSlider::intToDouble(int value)
{
    double val = static_cast<double>(value);
    val /= 100;

    emit valueChanged(val);
}

void QDSlider::setMaximum(double value)
{
    value = QString::number(value, 'f', 2).toDouble() * 100;

    QSlider::setMaximum(value);
}

void QDSlider::setValue(double value)
{
    value = QString::number(value, 'f', 2).toDouble() * 100;

    QSlider::setValue(value);
}

double QDSlider::value() const
{
    return QSlider::value() / 100;
}
