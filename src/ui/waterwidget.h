#ifndef WATERWIDGET_H
#define WATERWIDGET_H

#include <QDialog>

#include "waterchunk.h"

namespace Ui
{
    class WaterWidget;
}

class WaterWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WaterWidget(QWidget* parent = 0);
    ~WaterWidget();

private:
    Ui::WaterWidget* ui;

    WaterChunk* waterChunk;

private slots:
    void setWaterStatus(int status);

    void setTopHeight(double value);
    void setRightHeight(double value);
    void setBottomHeight(double value);
    void setLeftHeight(double value);

    void updateWater();

public slots:
    void setChunk(WaterChunk* chunk);
};

#endif // WATERWIDGET_H
