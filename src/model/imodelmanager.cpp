#include "imodelmanager.h"

IModelManager::IModelManager()
{
    textureManager = new ITextureManager();
    current = -1;
}

IModelManager::~IModelManager()
{
    QList<IModelData*> models = data.values();
    for(int i = 0; i < models.size(); ++i)
    {
        delete models.at(i);
    }
    delete textureManager;
}

bool IModelManager::loadModel(QString category, QString file)
{
    if(data.keys().contains(file))
        return false;

    IModelInterface *model = 0;

    try
    {
        model = new IModelInterface(textureManager,file);
    }
    catch(...)
    {
        delete model;
        return false;
    }
    data.insert(file,new IModelData(category,QImage("://object_icon"),model));

    if(!categories.contains(category))
        categories.push_back(category);

    return true;
}

void IModelManager::loadModels(QString modelsDirectory)
{
    QDir dirModels(modelsDirectory + "/models");
    QStringList categoryList = dirModels.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i < categoryList.size(); ++i)
    {
        QString category = categoryList.at(i);
        QString categoryPath = dirModels.path() + "/" + category;
        QDir dirCategory(categoryPath);
        QStringList fileNames = dirCategory.entryList(QDir::Files);
        for(int j = 0; j < fileNames.size(); ++j)
        {
            QString file = fileNames.at(j);

            if(file.contains(".mtl"))
                continue;

            QString filePath = dirModels.path() + "/" + category + "/" + file;
            loadModel(category,filePath);
        }
    }
}

QVector<QString> IModelManager::getCategories()
{
    return categories;
}

QVector<QString> IModelManager::getNames()
{
    return data.keys().toVector();
}

QVector<QString> IModelManager::getNames(QString category)
{
    QVector<QString> names;
    for(int i = 0; i < data.size(); ++i)
    {
        if(data.values().at(i)->category == category)
            names.push_back(data.keys().at(i));
    }
    return names;
}

int IModelManager::getIndex(QString modelPath)
{
    for(int i = 0; i < data.keys().size(); ++i)
    {
        if(data.keys().at(i) == modelPath)
            return i;
    }
    return -1;
}

IModelData *IModelManager::getModel(int index)
{
    return data.values().at(index);
}

ITextureManager *IModelManager::getTextureManager()
{
    return textureManager;
}
