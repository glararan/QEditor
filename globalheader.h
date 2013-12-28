#ifndef GLOBALHEADER_H
#define GLOBALHEADER_H

#include <QtGlobal>

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

#endif // GLOBALHEADER_H