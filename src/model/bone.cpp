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

#include "bone.h"

Bone::Bone(const int bone_id, const QMatrix4x4 bone_offset)
{
    id     = bone_id;
    offset = bone_offset;
    parent = NULL;
}

void Bone::addChild(Bone* child)
{
    child->setParent(this);

    children.push_back(child);
}

Bones::Bones()
{
}

void Bones::addBone(const QString name, Bone* bone)
{
    bones.insert(name, bone);
}

Bone* Bones::getBone(int id) const
{
    for(int i = 0; i < bones.values().size(); ++i)
    {
        if(bones.values().at(i)->getId() == id)
            return bones.values().at(i);
    }

    return 0;
}

Bone* Bones::createEmptyBone(const QString name, const QMatrix4x4 offset)
{
    Bone* bone = new Bone(bones.size(), offset);

    addBone(name, bone);

    return bone;
}
