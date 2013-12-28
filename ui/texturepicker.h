#ifndef TEXTUREPICKER_H
#define TEXTUREPICKER_H

#include <QDialog>

#include "tableview.h"
#include "texturemanager.h"

#include <QVector>
#include <QPair>
#include <QImage>
#include <QTableWidgetItem>

class MapChunk;

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

    void initialize(TextureManager* manager);
    
protected:
    void resizeEvent(QResizeEvent* e);

private:
    Ui::TexturePicker* ui;

    TextureManager* textureManager;

    TextureWell* textureWell;

    MapChunk* mapChunk;

    QSize textureIconSize;
    QSize textureIconMargin;

    int columns;

    QVector<QPair<QImage, QString>> textures;

private slots:
    void selectedTexture(int row, int cell);

    void moveLayerUp();
    void moveLayerDown();
    void deleteLayer();

    void handleLayerGUI(QTableWidgetItem* item);

public slots:
    void setChunk(MapChunk* chunk);
};

#endif // TEXTUREPICKER_H
