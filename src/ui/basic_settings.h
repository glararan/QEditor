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

#ifndef BASIC_SETTINGS_H
#define BASIC_SETTINGS_H

#include <QWizard>

namespace Ui
{
    class Basic_Settings;
}

class World;

class Basic_Settings : public QWizard
{
    Q_OBJECT

public:
    Basic_Settings(World* mWorld, QWidget* parent = 0);
    ~Basic_Settings();

protected:
    void accept();
    void reject();

private:
    Ui::Basic_Settings* ui;

    World* world;

private slots:
    void setDefaultHeightEnabled(int state);

    void openTerrainGenerationWidget();
    void openTerrainImportExportWidget();

    void cancelEvent();

public slots:

signals:
    void openTerrainGeneration();
    void openTerrainImportExport();
};

#endif // BASIC_SETTINGS_H
