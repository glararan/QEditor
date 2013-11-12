#ifndef QEDITOR_H
#define QEDITOR_H

#include <QApplication>
#include <QSettings>

#include "mainwindow.h"

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
