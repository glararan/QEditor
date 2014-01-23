#ifndef MODELPICKER_H
#define MODELPICKER_H

#include <QWidget>
#include "model/imodelmanager.h"
#include "ui/tableview.h"
#include "ui/toolbox.h"

namespace Ui {
class ModelPicker;
}

class ModelPicker : public QWidget
{
    Q_OBJECT

public:
    explicit ModelPicker(QWidget *parent = 0);
    ~ModelPicker();
    void loadPicker(IModelManager *manager);
    void clear();

private:
    Ui::ModelPicker *ui;
    QString currentModelLocation;
    ToolBox *box;
    QVector<TextureWell*> items;
    IModelManager *manager;

private slots:
    void modelSelected(int, int);
    void modelviewSelected(int, int);
};

#endif // MODELPICKER_H
