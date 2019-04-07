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

#ifndef HEIGHTMAPWIDGET_H
#define HEIGHTMAPWIDGET_H

#include <QDialog>

#include <QTcpServer>
#include <QTcpSocket>

namespace Ui
{
    class HeightmapWidget;
}

/*class PhoneDataSocket : public QTcpSocket
{
    Q_OBJECT

public:
    enum ConnectionState
    {
        WaitingForGreeting,
        ReadyGreeting,
        ReadyForUse
    };

    enum DataType
    {
        MapData,
        Ping,
        Pong,
        Greeting,
        Undefined
    };

    PhoneDataSocket(QObject* parent = 0);

    void setGreetingMessage(const QString& message);

private:
    QString greetingMessage;

    QTimer pingTimer;
    QTime pongTime;

    QByteArray buffer;

    ConnectionState state;
    DataType currentDataType;

    int numBytesForCurrentDataType;
    int transferTimerId;

    bool isGreetingMessageSent;

    int readDataIntoBuffer(int maxSize = 1024000);
    int dataLengthForCurrentDataType();

    bool readProtocolHeader();
    bool hasEnoughData();

    void processData();

protected:
    void timerEvent(QTimerEvent* event) Q_DECL_OVERRIDE;

private slots:
    void processReadyRead();

    void sendPing();
    void sendGreetingMessage();

signals:
    void readyForUse();
    void newMapData(const QString& from, QString& data);
};*/
/*
class PhoneDataServer : public QTcpServer
{
    Q_OBJECT

public:
    PhoneDataServer(quint16 port, QObject* parent = 0);

signals:
    void newConnection(PhoneDataSocket* socket);

protected:
    void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;
};*/

class HeightmapWidget : public QDialog
{
    Q_OBJECT

public:
    explicit HeightmapWidget(QWidget* parent = 0);
    ~HeightmapWidget();

private:
    Ui::HeightmapWidget* ui;

    //PhoneDataServer* server;

private slots:
    void showImportBrowser();
    void showExportBrowser();
    void showSTLExportBrowser();

    void doImport();
    void doExport();
    void doSTLExport();

    void setScale(double scale);

    void serverSwitch();

signals:
    void importing(QString path, float scale);
    void exporting(QString path, float scale);
    void stlExporting(QString path, float surface, bool scaleHeight, bool low, bool tile);

    void setScale(float scale);
};

#endif // HEIGHTMAPWIDGET_H
