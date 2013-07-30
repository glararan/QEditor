#include <QApplication>
#include <QtDebug>
#include <QFile>
#include <QTextStream>

#include <stdio.h>
#include <stdlib.h>

#include "mainwindow.h"

void LogHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    QByteArray lMessage = message.toLocal8Bit();

    QString text = NULL;

    switch(type)
    {
        case QtDebugMsg:
            text = QString("Debug (%s:%u, %s): %1").arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
            break;

        case QtWarningMsg:
            text = QString("Warning (%s:%u, %s): %1").arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
            break;

        case QtCriticalMsg:
            text = QString("Critical (%s:%u, %s): %1").arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
            break;

        case QtFatalMsg:
            {
                text = QString("Fatal (%s:%u, %s): %1").arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
                abort();
            }
            break;
    }

    QFile file("log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream tStream(&file);
    tStream << text << endl;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    //qInstallMessageHandler(LogHandler);

    MainWindow mw;
    mw.showMaximized();

    return app.exec();
}
