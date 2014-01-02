#ifndef STARTUP_H
#define STARTUP_H

#include <QWidget>

#include "newproject.h"

namespace Ui
{
    class StartUp;
}

struct RecentProject
{
    QString projectName;
    QString projectPath;
    QString projectFile;

    operator QVariant() const
    {
        return QVariant::fromValue(*this);
    }

    bool operator==(const RecentProject& other) const
    {
        Q_ASSERT(this != &other);

        if(projectName == other.projectName && projectPath == other.projectPath && projectFile == other.projectFile)
            return true;
        else
            return false;
    }
};

Q_DECLARE_METATYPE(RecentProject)

class StartUp : public QWidget
{
    Q_OBJECT

public:
    explicit StartUp(QWidget* parent = 0);
    ~StartUp();

    static void addRecentProject(RecentProject& recentProject);

public slots:
    void showNewProject();
    void showOpenProject();

signals:
    void openProject(ProjectFileData projectData);

    void createMemoryProject(NewProjectData projectData);

private slots:
    void createProject(NewProjectData projectData);
    void openRecentProject(QString fileName);

protected:
    void resize(int w, int h);

private:
    Ui::StartUp* ui;

    NewProject* newProject;

    void openProject(QString path);

    void setMargins(int width);
};

inline QDataStream& operator<<(QDataStream& out, const RecentProject& recentProject)
{
    out << recentProject.projectName;
    out << recentProject.projectPath;
    out << recentProject.projectFile;

    return out;
}

inline QDataStream& operator>>(QDataStream& in, RecentProject& recentProject)
{
    in >> recentProject.projectName;
    in >> recentProject.projectPath;
    in >> recentProject.projectFile;

    return in;
}

#endif // STARTUP_H