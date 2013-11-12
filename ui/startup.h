#ifndef STARTUP_H
#define STARTUP_H

#include <QWidget>

#include "newproject.h"

namespace Ui
{
    class StartUp;
}

class StartUp : public QWidget
{
    Q_OBJECT

public:
    explicit StartUp(QWidget* parent = 0);
    ~StartUp();

public slots:
    void showNewProject();
    void showOpenProject();

signals:
    void openProject(ProjectFileData projectData);

    void createMemoryProject(NewProjectData projectData);

private slots:
    void createProject(NewProjectData projectData);

private:
    Ui::StartUp* ui;

    NewProject* newProject;
};

#endif // STARTUP_H
