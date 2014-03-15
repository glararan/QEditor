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
