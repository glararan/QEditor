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

#include "project_settings.h"
#include "ui_project_settings.h"

#include <QFileDialog>

Project_Settings::Project_Settings(ProjectFileData& projectData, QWidget* parent)
: QDialog(parent)
, ui(new Ui::Project_Settings)
, projectSettings(projectData)
{
    ui->setupUi(this);

    ui->fileNameTextBox->setText(projectData.projectFile);
    ui->pathTextBox->setText(projectData.projectRootDir);
    ui->projectNameTextBox->setText(projectData.projectName);
    ui->mapNameTextBox->setText(projectData.mapName);

    connect(ui->pathButton, SIGNAL(clicked()), this, SLOT(projectDir()));
    connect(ui->buttonBox,  SIGNAL(accepted()), this, SLOT(setProjectData()));
}

Project_Settings::~Project_Settings()
{
    delete ui;
}

void Project_Settings::projectDir()
{
    ui->pathTextBox->setText(QFileDialog::getExistingDirectory(this, tr("Choose project root directory"), QString(), QFileDialog::ShowDirsOnly |
                                                                                                                     QFileDialog::DontResolveSymlinks));
}

void Project_Settings::setProjectData()
{
    projectSettings.projectFile    = ui->fileNameTextBox->text();
    projectSettings.projectRootDir = ui->pathTextBox->text();
    projectSettings.projectName    = ui->projectNameTextBox->text();
    projectSettings.mapName        = ui->mapNameTextBox->text();

    emit projectDataChanged(projectSettings);
}