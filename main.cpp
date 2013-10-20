#include <QApplication>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QTime>

#include "mainwindow.h"

void LogHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    QByteArray lMessage = message.toLocal8Bit();

    QString text = NULL;

    switch(type)
    {
        case QtDebugMsg:
            text = QString("Debug (%1:%2, %3): %4").arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
            break;

        case QtWarningMsg:
            text = QString("Warning (%1:%2, %3): %4").arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
            break;

        case QtCriticalMsg:
            text = QString("Critical (%1:%2, %3): %4").arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
            break;

        case QtFatalMsg:
            {
                text = QString("Fatal (%1:%2, %3): %4").arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
                abort();
            }
            break;
    }

    QFile file("log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream tStream(&file);
    tStream << text << endl;

    file.close();
}

void LogInitialize()
{
    /// truncate log
    QFile file("log.txt");
    file.open(QIODevice::WriteOnly);
    file.resize(0);
    file.close();

    /// install log
    qInstallMessageHandler(LogHandler);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    LogInitialize();

    qsrand((uint)QTime::currentTime().msec());

    MainWindow mw;
    mw.showMaximized();

    return app.exec();
}
