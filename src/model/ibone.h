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

#ifndef IBONE_H
#define IBONE_H

#include <QtWidgets>

class IBone
{
public:
    ~IBone();

    void addChild(IBone* child);

    void setId(int bone_id);
    void setParent(IBone* bone_parent);
    void setNodeTransformation(QMatrix4x4 bone_nodeTransformation);

    int getId();

    QMatrix4x4 getOffset();
    QMatrix4x4 getNodeTransformation();

    IBone*           getParent();
    QVector<IBone*>* getChildren();

    friend class IBones; // move to private?

private:
    IBone(int bone_id, QMatrix4x4 bone_offset = QMatrix4x4()); // shouldn't be in public?

    IBone* parent;

    int id;

    QMatrix4x4 offset;
    QMatrix4x4 nodeTransformation;

    QVector<IBone*> children;
};

class IBones
{
public:
    IBones();

    IBone* createEmptyBone(QString name, QMatrix4x4 offset = QMatrix4x4());

    void addBone(QString name, IBone* bone);

    bool hasBone(QString name);

    IBone* getBone(QString name);
    IBone* getBone(int id);

    QVector<QString> getBoneNames();

    int getBoneCount();

private:
    QMap<QString, IBone*> bones;
};

#endif // IBONE_H