#ifndef TEXTUREPICKER_H
#define TEXTUREPICKER_H

#include <QDialog>

#include "tableview.h"

#include <QVector>
#include <QPair>
#include <QImage>
#include <QGridLayout>

namespace Ui
{
    class TexturePicker;
}

class TexturePicker : public QDialog
{
    Q_OBJECT
    
public:
    explicit TexturePicker(QWidget* parent = 0);
    ~TexturePicker();

    const QImage getTexture() const;
    
protected:
    void resizeEvent(QResizeEvent* e);

private:
    Ui::TexturePicker* ui;

    QGridLayout* layout;

    TextureWell* textureWell;

    QSize textureIconSize;
    QSize textureIconMargin;

    int columns;

    QVector<QPair<QImage, QString>> textures;
};

#endif // TEXTUREPICKER_H
