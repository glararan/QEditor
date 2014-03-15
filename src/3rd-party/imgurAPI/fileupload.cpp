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

#include "fileupload.h"

#include <QtNetwork>
#include <QNetworkRequest>
#include <QDomDocument>
#include <QDebug>
#include <QImage>
#include <QBuffer>
#include <QUrl>

FileUpload::FileUpload(QObject* parent)
: QObject(parent)
, manager(new QNetworkAccessManager(this))
{
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
}

void FileUpload::uploadImage(const QImage& image)
{
    if(image.isNull())
        return;

    QByteArray imageData;

    QBuffer buffer(&imageData);

    bool open = buffer.open(QIODevice::WriteOnly);
    bool save = image.save(&buffer, "PNG");

    if(!open || !save)
    {
        emit uploadError(tr("Unable to read image."));

        return;
    }

    QByteArray requestBody;
    requestBody.append(QString("?type=base64").toUtf8());
    requestBody.append(QString("&image=").toUtf8());
    requestBody.append(QUrl::toPercentEncoding(imageData.toBase64()));

    QNetworkRequest request;
    request.setUrl(QUrl("https://api.imgur.com/3/upload.xml"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setRawHeader("Authorization", IMGUR_ClientID);

    manager->post(request, requestBody);
}

void FileUpload::requestFinished(QNetworkReply* reply)
{
    if(reply->error() != QNetworkReply::NoError)
    {
        emit uploadError(tr("Network error: %1").arg(reply->error()));

        reply->deleteLater();

        return;
    }

    QDomDocument doc;

    QString link;
    QString error;

    if(!doc.setContent(reply->readAll(), false, &error))
    {
        emit uploadError(tr("Parse error: %1").arg(error));

        reply->deleteLater();

        return;
    }

    QDomElement rootElement = doc.documentElement();

    error = "";

    if(rootElement.tagName() == "data")
    {
       QDomNode node = rootElement.firstChild();

       while(!node.isNull())
       {
           if(node.isElement() && node.toElement().tagName() == "link")
               link = node.toElement().text();

           node = node.nextSibling();
       }
    }
    else if(rootElement.tagName() == "data")
    {
        QDomNode node = rootElement.firstChild();

        while(!node.isNull())
        {
            if(node.isElement() && node.toElement().tagName() == "message")
                error = node.toElement().text();

            node = node.nextSibling();
        }
    }
    else
        error = tr("Reccived unexpected reply from web service");

    if(!error.isEmpty())
        emit uploadError(error);
    else
        emit uploadDone(link);

    reply->deleteLater();
}