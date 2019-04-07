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

#include "qeditor.h"

#include "version.h"
#include "mainwindow.h"

#include <QTime>
#include <QTranslator>

QEditor::QEditor(int& argc, char** argv)
: QApplication(argc, argv)
{
    setAttribute(Qt::AA_ShareOpenGLContexts);

    setOrganizationDomain(QString(VER_COMPANYDOMAIN_STR));
    setOrganizationName(QString(VER_COMPANYNAME_STR));
    setApplicationName(QString(VER_PRODUCTNAME_STR));
    setApplicationVersion(QString(VER_FILEVERSION_STR));

    settings = new QSettings(this);

    translator = new QTranslator(this);
    translator->load(getSetting("language", QLocale::system().name()).toString(), ":/languages/", QString(), ".qlg");

    installTranslator(translator);

    qsrand((uint)QTime::currentTime().msec());

    /// CAUSING DISABLING 3D picking
    /*QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);

    QSurfaceFormat::setDefaultFormat(format);*/

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
    delete settings;
    delete translator;

    delete mainWindow;
}

QVariant QEditor::getSetting(const QString& key, const QVariant& value) const
{
    return settings->value(key, value);
}

QString QEditor::getGraphics() const
{
    return graphics;
}

void QEditor::setSetting(const QString& key, const QVariant& value)
{
    settings->setValue(key, value);
    settings->sync();
}

void QEditor::setGraphics(const QString& graphicsVendor)
{
    graphics = graphicsVendor;
}

QEditor& app()
{
    return *reinterpret_cast<QEditor*>(qApp);
}
