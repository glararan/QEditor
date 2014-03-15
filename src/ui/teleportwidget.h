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

#ifndef TELEPORTWIDGET_H
#define TELEPORTWIDGET_H

#include <QDialog>

namespace Ui
{
    class TeleportWidget;
}

class TeleportWidget : public QDialog
{
    Q_OBJECT
    
public:
    explicit TeleportWidget(QWidget* parent = 0);
    ~TeleportWidget();
    
private:
    Ui::TeleportWidget* ui;

    bool checkCoords(QString coords);

    void writeToBookmarks(QString name, QVector3D location);
    bool readFromBookmarks(QString search, QVector3D& location);

    void loadBookmarks();

public slots:
    void bookmarksAdd();
    void bookmarksGo();
    void bookmarksTeleport();

signals:
    void TeleportTo(QVector3D* location);
};

#endif // TELEPORTWIDGET_H