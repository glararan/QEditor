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
    QSettings* settings;

    MainWindow* mainWindow;

    QString graphics;

signals:

public slots:

};

QEditor& app();

#endif // QEDITOR_H