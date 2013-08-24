#ifndef QDSLIDER_H
#define QDSLIDER_H

#include <QSlider>
#include <QtWidgets/QWidget>

class QDSlider : public QSlider
{
    Q_OBJECT

public:
    QDSlider(int decimals = 2, QWidget* parent = 0);

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
