#include "qeditor.h"

#include "mainwindow.h"

#include <QTime>

QEditor::QEditor(int& argc, char** argv)
: QApplication(argc, argv)
{
    setOrganizationDomain("https://github.com/glararan/QEditor");
    setOrganizationName("");
    setApplicationName("QEditor");
    setApplicationVersion("0.1 pre-alpha");

    settings = new QSettings(this);

    qsrand((uint)QTime::currentTime().msec());

    mainWindow = new MainWindow();
    mainWindow->showMaximized();
}

QEditor::~QEditor()
{
    delete mainWindow;
}

QVariant QEditor::getSetting(const QString& key, const QVariant& value) const
{
    return settings->value(key, value);
}

void QEditor::setSetting(const QString& key, const QVariant& value)
{
    settings->setValue(key, value);
    settings->sync();
}

QEditor& app()
{
    return *reinterpret_cast<QEditor*>(qApp);
}
