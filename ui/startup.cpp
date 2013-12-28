#include "startup.h"
#include "ui_startup.h"

#include "qeditor.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVariantList>
#include <QVBoxLayout>

StartUp::StartUp(QWidget* parent)
: QWidget(parent)
, ui(new Ui::StartUp)
, newProject(new NewProject())
{
    ui->setupUi(this);

    qRegisterMetaTypeStreamOperators<RecentProject>("RecentProject");

    /// Recent projects
    QVariantList recentProjects = app().getSetting("recentProjects").toList();

    int projIndex = 0;

    foreach(QVariant proj, recentProjects)
    {
        RecentProject project = proj.value<RecentProject>();

        QLabel* label = new QLabel(QString("<html>"
                                           "   <head>"
                                           "       <style type=text/css>"
                                           "           a       {color:rgb(0, 128, 255);text-decoration:none;}"
                                           "           a:hover {text-decoration:underline;}" // Qt doesn't support :hover

                                           "           span    {color:gray;}"
                                           "       </style>"
                                           "   </head>"
                                           "   <body>"
                                           "       <ul><li><a href='%1'>%2<br><span>%1</span></<a><br></li></ul>"
                                           "   </body>"
                                           "</html>"
                                          ).arg(project.projectPath + "/" + project.projectFile + ".qep").arg(project.projectName));

        label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

        connect(label, SIGNAL(linkActivated(QString)), this, SLOT(openRecentProject(QString)));

        ui->verticalLayout_6->addWidget(label, projIndex++);
    }

    ui->verticalLayout_6->addStretch(++projIndex);

    setMargins(width());

    // buttons
    connect(ui->pushButton,   SIGNAL(clicked()), this, SLOT(showNewProject()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(showOpenProject()));

    // new project
    connect(newProject, SIGNAL(wizardData(NewProjectData)),
            this,      SLOT(createProject(NewProjectData)));
}

StartUp::~StartUp()
{
    delete ui;
}

void StartUp::showNewProject()
{
    newProject->show();
}

void StartUp::showOpenProject()
{
    openProject(QFileDialog::getOpenFileName(0, tr("Open Project"), QString(), tr("QEditor project files (*.qep)")));
}

void StartUp::createProject(NewProjectData projectData)
{
    emit createMemoryProject(projectData);
}

void StartUp::openProject(QString path)
{
    QFile file(path);

    activateWindow();

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        ProjectFileData data;

        QDataStream dataStream(&file);
        dataStream.setVersion(QDataStream::Qt_5_0);

        dataStream >> data;

        if(data.version > PROJECT_VERSION)
        {
            QMessageBox msg;
            msg.setWindowTitle("Error");
            msg.setText(tr("Project file is newer and we don't have needed structure for load it into this version!"));
            msg.setIcon(QMessageBox::Critical);

            msg.exec();

            return;
        }

        RecentProject recentProject;
        recentProject.projectFile = data.projectFile;
        recentProject.projectName = data.projectName;
        recentProject.projectPath = data.projectRootDir;

        addRecentProject(recentProject);

        emit openProject(data);
    }
    else
    {
        QMessageBox msg;
        msg.setWindowTitle("Error");
        msg.setText(tr("Project file %1 doesn't exists!").arg(path));
        msg.setIcon(QMessageBox::Critical);

        msg.exec();
    }
}

void StartUp::openRecentProject(QString fileName)
{
    openProject(fileName);
}

void StartUp::addRecentProject(RecentProject& recentProject)
{
    QVariantList recentProjects = app().getSetting("recentProjects").toList();

    // find recentProject
    bool found = false;

    for(int i = 0; i < recentProjects.count(); ++i)
    {
        RecentProject rProj = recentProjects.at(i).value<RecentProject>();

        if(rProj == recentProject)
        {
            recentProjects.removeAt(i);

            found = true;

            break;
        }
    }

    if(recentProjects.count() >= 9 && found)
    {
        while(recentProjects.count() > 9)
            recentProjects.removeLast();
    }

    recentProjects.insert(0, QVariant::fromValue(recentProject));

    app().setSetting("recentProjects", recentProjects);
}

void StartUp::resize(int w, int h)
{
    setMargins(w);
}

void StartUp::setMargins(int width)
{
    int top, right, bottom, left;

    ui->gridLayout->getContentsMargins(&left, &top, &right, &bottom);

    right = left = MathHelper::toInt(MathHelper::toDouble(width) / 100.0 * 50.0);

    ui->gridLayout->setContentsMargins(left, top, right, bottom);
}