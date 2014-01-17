#include <QtDebug>
#include <QFile>
#include <QTextStream>

#include "qeditor.h"

void LogHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    QByteArray lMessage = message.toLocal8Bit();

    QString text = NULL;

    switch(type)
    {
        case QtDebugMsg:
            text = QString(QObject::tr("Debug (%1:%2, %3): %4")).arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
            break;

        case QtWarningMsg:
            text = QString(QObject::tr("Warning (%1:%2, %3): %4")).arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
            break;

        case QtCriticalMsg:
            text = QString(QObject::tr("Critical (%1:%2, %3): %4")).arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
            break;

        case QtFatalMsg:
            {
                text = QString(QObject::tr("Fatal (%1:%2, %3): %4")).arg(context.file).arg(context.line).arg(context.function).arg(lMessage.constData());
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
    //LogInitialize();

    QEditor application(argc, argv);

    return application.exec();
}