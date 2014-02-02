#ifndef IBONE_H
#define IBONE_H

#include <QtWidgets>

class IBone
{
public:
    friend class IBones;
    ~IBone();
    void setId(int id);
    int getId();
    QMatrix4x4 getOffset();

    void setNodeTransformation(QMatrix4x4 nodeTransformation);
    QMatrix4x4 getNodeTransformation();

    void setParent(IBone *parent);
    void addChild(IBone *child);

    IBone *getParent();
    QVector<IBone *> *getChildren();

private:
    IBone(int id, QMatrix4x4 offset = QMatrix4x4());
    int id;
    QMatrix4x4 offset;
    IBone *parent;
    QVector<IBone *> children;
    QMatrix4x4 nodeTransformation;
};

class IBones
{
public:
    IBones();
    void addBone(QString name, IBone *bone);
    bool hasBone(QString name);
    IBone* getBone(QString name);
    IBone* getBone(int id);
    QVector<QString> getBoneNames();
    int getBoneCount();
    IBone* createEmptyBone(QString name, QMatrix4x4 offset = QMatrix4x4());
private:
    QMap<QString, IBone *> bones;
};

#endif // IBONE_H
