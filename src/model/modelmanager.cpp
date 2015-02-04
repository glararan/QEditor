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

#include "modelmanager.h"

#include "model.h"

ModelManager::ModelManager(TextureManager* manager)
: textureManager(manager)
, current(-1)
{
}

ModelManager::~ModelManager()
{
    QList<ModelData*> modelsData = data.values();

    for(int i = 0; i < modelsData.size(); ++i)
        delete modelsData.at(i);

    QList<Model*> modelsList = models.values();

    for(int i = 0; i < models.size(); ++i)
        delete modelsList.at(i);
}

bool ModelManager::loadModel(const QString category, const QString file)
{
    if(data.keys().contains(file))
        return false;

    ModelInterface* model = 0;

    try
    {
        model = new ModelInterface(textureManager, file);
    }
    catch(...)
    {
        delete model;

        return false;
    }

    data.insert(file, new ModelData(category, QImage("://object_icon"), model));
    models.insert(file, new Model(this, model));

    if(!categories.contains(category))
        categories.push_back(category);

    return true;
}

void ModelManager::loadModels(const QString modelsDirectory)
{
    QDir dirModels(modelsDirectory + "/models");

    QStringList categoryList = dirModels.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    for(int i = 0; i < categoryList.size(); ++i)
    {
        QString category     = categoryList.at(i);
        QString categoryPath = dirModels.path() + "/" + category;

        QDir dirCategory(categoryPath);

        QStringList fileNames = dirCategory.entryList(QDir::Files);

        for(int j = 0; j < fileNames.size(); ++j)
        {
            QString file = fileNames.at(j);

            if(file.contains(".mtl"))
                continue;

            QString filePath = dirModels.path() + "/" + category + "/" + file;

            loadModel(category, filePath);
        }
    }
}

const QVector<QString> ModelManager::getNames(const QString category) const
{
    QVector<QString> names;

    for(int i = 0; i < data.size(); ++i)
    {
        if(data.values().at(i)->category == category)
            names.push_back(data.keys().at(i));
    }

    return names;
}

const int ModelManager::getIndex(const QString modelPath) const
{
    for(int i = 0; i < data.keys().size(); ++i)
    {
        if(data.keys().at(i) == modelPath)
            return i;
    }

    return -1;
}
