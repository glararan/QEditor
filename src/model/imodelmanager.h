#ifndef IMODELMANAGER_H
#define IMODELMANAGER_H

#include <QtCore>
#include "imodelinterface.h"
#include "itexturemanager.h"

struct IModelData
{
    IModelData(QString category, QImage image, IModelInterface *modelInterface)
    {
        this->category = category;
        this->image = image;
        this->modelInterface = modelInterface;
    }
    ~IModelData()
    {
        delete modelInterface;
    }

    QString category;
    QImage image;
    IModelInterface *modelInterface;
};

class IModelManager
{
public:
    IModelManager();
    ~IModelManager();
    bool loadModel(QString category, QString file);
    void loadModels(QString modelsDirectory);
    QVector<QString> getCategories();
    QVector<QString> getNames();
    QVector<QString> getNames(QString category);
    int getIndex(QString modelPath);
    IModelData *getModel(int index);
    ITextureManager *getTextureManager();
    int getCurrentModel() { return current; }
    void setCurrentModel(int index) { current = index; }
    bool isModelSelected() { return current >= 0; }

private:
    QMap<QString, IModelData*> data;
    QVector<QString> categories;
    ITextureManager *textureManager;
    int current;
};

#endif // IMODELMANAGER_H
