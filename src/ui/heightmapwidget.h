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

#ifndef HEIGHTMAPWIDGET_H
#define HEIGHTMAPWIDGET_H

#include <QDialog>

namespace Ui
{
    class HeightmapWidget;
}

class HeightmapWidget : public QDialog
{
    Q_OBJECT

public:
    explicit HeightmapWidget(QWidget* parent = 0);
    ~HeightmapWidget();

private:
    Ui::HeightmapWidget* ui;

private slots:
    void showImportBrowser();
    void showExportBrowser();
    void showSTLExportBrowser();

    void doImport();
    void doExport();
    void doSTLExport();

    void setScale(double scale);

signals:
    void importing(QString path, float scale);
    void exporting(QString path, float scale);
    void stlExporting(QString path, float surface, bool scaleHeight, bool low, bool tile);

    void setScale(float scale);
};

#endif // HEIGHTMAPWIDGET_H
