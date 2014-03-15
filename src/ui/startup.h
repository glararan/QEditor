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