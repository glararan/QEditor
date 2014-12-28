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

#ifndef BONE_H
#define BONE_H

#include <QtWidgets>

class Bone
{
public:
    ~Bone();

    void addChild(Bone* child);

    void setId(const int bone_id)                                        { id = bone_id; }
    void setParent(Bone* bone_parent)                                    { parent = bone_parent; }
    void setNodeTransformation(const QMatrix4x4 bone_nodeTransformation) { nodeTransformation = bone_nodeTransformation; }

    const int getId() const { return id; }

    const QMatrix4x4 getOffset() const             { return offset; }
    const QMatrix4x4 getNodeTransformation() const { return nodeTransformation; }

    Bone*           getParent() const         { return parent; }
    const QVector<Bone*>* getChildren() const { return &children; }

    friend class Bones; // move to private?

private:
    Bone(const int bone_id, const QMatrix4x4 bone_offset = QMatrix4x4()); // shouldn't be in public?

    Bone* parent;

    int id;

    QMatrix4x4 offset;
    QMatrix4x4 nodeTransformation;

    QVector<Bone*> children;
};

class Bones
{
public:
    Bones();

    Bone* createEmptyBone(const QString name, const QMatrix4x4 offset = QMatrix4x4());

    void addBone(const QString name, Bone* bone);

    const bool hasBone(const QString name) const { return bones.keys().contains(name); }

    Bone* getBone(const QString name) const { return bones.value(name); }
    Bone* getBone(const int id) const;

    const QVector<QString> getBoneNames() const { return bones.keys().toVector(); }

    const int getBoneCount() const { return bones.size(); }

private:
    QMap<QString, Bone*> bones;
};

#endif // BONE_H
