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

#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QDialog>

#include "camera.h"

namespace Ui
{
    class CameraWidget;
}

class CameraWidget : public QDialog
{
    Q_OBJECT

public:
    explicit CameraWidget(Camera* cam, QWidget* parent = 0);
    ~CameraWidget();

private:
    Ui::CameraWidget* ui;

    Camera* camera;

    bool playing;

    enum CameraTableColumns
    {
        Index     = 0,
        PositionX = 1,
        PositionY = 2,
        PositionZ = 3,
        Count
    };

    QVector<QVector3D> viewCenter;
    QVector<QVector3D> upVector;

    void generateBezierCurves(int row);

public slots:
    void selectPoint(const QVector3D& position);

private slots:
    void itemChanged(QTableWidgetItem* item);

    void setShowPath(int state);
    void setRepeat(int state);

    void addPosition();
    void setPosition();
    void deletePosition();

    void play();
    void stop();
    void clear();

signals:
    void showPath(bool);
    void repeatPlay(bool);

    void selectedPoint(QVector<QTableWidgetItem*>& item);
};

#endif // CAMERAWIDGET_H
