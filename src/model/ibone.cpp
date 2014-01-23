#include "ibone.h"

IBone::IBone(int id, QMatrix4x4 offset)
{
    this->id = id;
    this->offset = offset;
    parent = NULL;
}

void IBone::setId(int id)
{
    this->id = id;
}

int IBone::getId()
{
    return id;
}

QMatrix4x4 IBone::getOffset()
{
    return offset;
}

void IBone::setNodeTransformation(QMatrix4x4 nodeTransformation)
{
    this->nodeTransformation = nodeTransformation;
}

QMatrix4x4 IBone::getNodeTransformation()
{
    return nodeTransformation;
}

void IBone::setParent(IBone *parent)
{
    this->parent = parent;
}

void IBone::addChild(IBone *child)
{
    child->setParent(this);
    children.push_back(child);
}

IBone *IBone::getParent()
{
    return parent;
}

QVector<IBone *> *IBone::getChildren()
{
    return &children;
}


IBones::IBones()
{

}

void IBones::addBone(QString name, IBone *bone)
{
    bones.insert(name,bone);
}

bool IBones::hasBone(QString name)
{
    return bones.keys().contains(name);
}

IBone *IBones::getBone(QString name)
{
    return bones.value(name);
}

IBone *IBones::getBone(int id)
{
    for(int i = 0; i < bones.values().size(); ++i)
    {
        if(bones.values().at(i)->getId() == id)
            return bones.values().at(i);
    }
    return 0;
}

QVector<QString> IBones::getBoneNames()
{
    return bones.keys().toVector();
}

int IBones::getBoneCount()
{
    return bones.size();
}

IBone *IBones::createEmptyBone(QString name, QMatrix4x4 offset)
{
    IBone *bone = new IBone(bones.size(),offset);
    addBone(name,bone);
    return bone;
}
