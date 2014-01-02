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