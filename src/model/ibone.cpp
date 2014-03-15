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
