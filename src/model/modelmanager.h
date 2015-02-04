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

#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <QtCore>

#include "modelinterface.h"
#include "texturemanager.h"

class Model;

struct ModelData
{
    ModelData(const QString Category, const QImage Image, ModelInterface* ModelInterface)
    {
        category       = Category;
        image          = Image;
        modelInterface = ModelInterface;
    }

    ~ModelData()
    {
        delete modelInterface;
    }

    QString category;
    QImage image;

    ModelInterface* modelInterface;
};

class ModelManager
{
public:
    explicit ModelManager(TextureManager* manager);
    ~ModelManager();

    const bool isModelSelected() const { return current >= 0; }

    bool loadModel(const QString category, const QString file);
    void loadModels(const QString modelsDirectory);

    const QVector<QString> getCategories() const { return categories; }
    const QVector<QString> getNames() const      { return data.keys().toVector(); }
    const QVector<QString> getNames(const QString category) const;

    const int getIndex(const QString modelPath) const;
    const int getCurrentModel() const { return current; }

    Model*          getModel(const int index) const     { return models.values().at(index); }
    ModelData*      getModelData(const int index) const { return data.values().at(index); }
    TextureManager* getTextureManager() const           { return textureManager; }

    void setCurrentModel(const int index) { current = index; }

private:
    TextureManager* textureManager;

    QMap<QString, Model*>     models;
    QMap<QString, ModelData*> data;
    QVector<QString>          categories;

    int current;
};

#endif // MODELMANAGER_H
