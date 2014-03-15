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