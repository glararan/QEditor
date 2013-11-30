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

class QEditor : public QApplication
{
    Q_OBJECT

public:
    QEditor(int& argc, char** argv);
    ~QEditor();

    QVariant getSetting(const QString& key, const QVariant& value = QVariant()) const;

    void setSetting(const QString& key, const QVariant& value);

private:
    QSettings* settings;

    MainWindow* mainWindow;

signals:

public slots:

};

QEditor& app();

#endif // QEDITOR_H
