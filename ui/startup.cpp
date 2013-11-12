#include "startup.h"
#include "ui_startup.h"

#include <QFileDialog>
#include <QMessageBox>

StartUp::StartUp(QWidget* parent)
: QWidget(parent)
, ui(new Ui::StartUp)
, newProject(new NewProject())
{
    ui->setupUi(this);

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
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open Project"), QString(), tr("QEditor project files (*.qep)"));

    activateWindow();

    QFile file(fileName);

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

        emit openProject(data);
    }
    else
    {
        QMessageBox msg;
        msg.setWindowTitle("Error");
        msg.setText(tr("Project file %1 doesn't exists!").arg(fileName));
        msg.setIcon(QMessageBox::Critical);

        msg.exec();
    }
}

void StartUp::createProject(NewProjectData projectData)
{
    emit createMemoryProject(projectData);
}
