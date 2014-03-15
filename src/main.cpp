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
#ifdef RELEASE_MODE
    LogInitialize();
#endif

    QEditor application(argc, argv);

    return application.exec();
}