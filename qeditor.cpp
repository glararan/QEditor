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

    qDebug() << "Compiler C++ version:" << __cplusplus;

#if (defined(ENVIRONMENT64))
    qDebug() << "Target architecture: 64-bit";
#else
    qDebug() << "Target architecture: 32-bit";
#endif

#if (defined(_MSC_BUILD) && defined(_MSC_FULL_VER) && defined(_MSC_VER))
    qDebug() << "MSVC build:" << _MSC_BUILD;
    qDebug() << "MSVC full_ver:" << _MSC_FULL_VER;
    qDebug() << "MSVC ver:" << _MSC_VER;
#endif

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