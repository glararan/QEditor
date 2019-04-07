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

#ifndef GLOBALHEADER_H
#define GLOBALHEADER_H

#include <QtGlobal>
#include <QVector2D>

#include "mathhelper.h"

const int PROJECT_VERSION    = 1;
const int MAP_HEADER_VERSION = 1;

struct NewProjectData
{
    QString projectFile;
    QString projectDir;
    QString projectName;
    QString mapName;

    bool** mapCoords;
};

inline uint qHash(const QVector2D& key)
{
    return qHash(key.length());
}

#endif // GLOBALHEADER_H
