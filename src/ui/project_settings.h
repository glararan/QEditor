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

#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

#include <QDialog>

#include "mapheaders.h"

namespace Ui
{
    class Project_Settings;
}

class Project_Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Project_Settings(ProjectFileData& projectData, QWidget* parent = 0);
    ~Project_Settings();

private:
    Ui::Project_Settings* ui;

    ProjectFileData& projectSettings;

private slots:
    void projectDir();
    void setProjectData();

signals:
    void projectDataChanged(ProjectFileData& data);
};

#endif // PROJECT_SETTINGS_H