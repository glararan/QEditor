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

#ifndef QEDITOR_H
#define QEDITOR_H

#include <QApplication>
#include <QSettings>

#include "mainwindow.h"

// Check Windows
#if _WIN32 || _WIN64
    #if _WIN64
        #define ENVIRONMENT64
    #else
        #define ENVIRONMENT32
    #endif
#endif

// Check GCC
#if __GNUC__
    #if __x86_64__ || __ppc64__
        #define ENVIRONMENT64
    #else
        #define ENVIRONMENT32
    #endif
#endif

// Define graphics vendors + current graphics
#ifndef AMD_VENDOR
    #define AMD_VENDOR "ATI Technologies Inc."
#endif

#ifndef NVIDIA_VENDOR
    #define NVIDIA_VENDOR "NVIDIA Corporation"
#endif

class QEditor : public QApplication
{
    Q_OBJECT

public:
    QEditor(int& argc, char** argv);
    ~QEditor();

    QVariant getSetting(const QString& key, const QVariant& value = QVariant()) const;
    QString  getGraphics() const;

    void setSetting(const QString& key, const QVariant& value);
    void setGraphics(const QString& graphicsVendor);

private:
    QSettings*   settings;
    QTranslator* translator;

    MainWindow* mainWindow;

    QString graphics;
};

QEditor& app();

#endif // QEDITOR_H