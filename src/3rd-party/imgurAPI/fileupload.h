#ifndef FILEUPLOAD_H
#define FILEUPLOAD_H

#include <QObject>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#define IMGUR_ClientID "Client-ID 76263ab8aedb431"

class FileUpload : public QObject
{
    Q_OBJECT

public:
    FileUpload(QObject* parent = 0);

    void uploadImage(const QImage& image);

signals:
    void uploadDone(const QString&);
    void uploadError(const QString&);

private slots:
    void requestFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager;
};

#endif // FILEUPLOAD_H