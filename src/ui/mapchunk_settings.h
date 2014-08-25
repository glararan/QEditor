#ifndef MAPCHUNK_SETTINGS_H
#define MAPCHUNK_SETTINGS_H

#include <QDialog>

#include "ui/qdslider.h"

class MapChunk;

namespace Ui
{
    class MapChunk_Settings;
}

class MapChunk_Settings : public QDialog
{
    Q_OBJECT

public:
    explicit MapChunk_Settings(QWidget* parent = 0);
    ~MapChunk_Settings();

private:
    Ui::MapChunk_Settings* ui;

    MapChunk* chunk;

    // Texture scale
    QDSlider* textureScaleFarSlider0;
    QDSlider* textureScaleNearSlider0;

    QDSlider* textureScaleFarSlider1;
    QDSlider* textureScaleNearSlider1;

    QDSlider* textureScaleFarSlider2;
    QDSlider* textureScaleNearSlider2;

    QDSlider* textureScaleFarSlider3;
    QDSlider* textureScaleNearSlider3;

private slots:
    void setTextureScaleOpt(int option);
    void setTextureScaleFar(double scale);
    void setTextureScaleNear(double scale);

public slots:
    void setChunk(MapChunk* mapChunk);
};

#endif // MAPCHUNK_SETTINGS_H
