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

#ifndef ANIMATION_H
#define ANIMATION_H

#include <QtWidgets>

#include "bone.h"

class Animation
{
public:
    Animation(const QString animation_name, const float animation_duration, const int animation_id);
    ~Animation();

    void addBonePosition(const int bone_id, const float time, const QVector3D   position);
    void addBoneScaling(const int bone_id,  const float time, const QVector3D   scaling);
    void addBoneRotation(const int bone_id, const float time, const QQuaternion rotation);

    void registerBone(const int boneId);

    void setBoneCount(const int count) { bone_count = count; }

    const bool isRegistered(const int boneId) const { return boneRegister.contains(boneId); }

    const int getId() const        { return id; }
    const int getBoneCount() const { return bone_count; }

    const float getDuration() const { return duration; }

    const QString getName() const { return name; }

    const QVector<QMatrix4x4> getTransforms(const float time_stamp, QMap<int, QPair<float, int>>* position_state, QMap<int, QPair<float, int>>* scaling_state, QMap<int, QPair<float, int>>* rotation_state) const;

    const QMap<int, QVector<QPair<float, QVector3D>>>*   getPositions() const { return &positions; }
    const QMap<int, QVector<QPair<float, QVector3D>>>*   getScalings() const  { return &scalings; }
    const QMap<int, QVector<QPair<float, QQuaternion>>>* getRotations() const { return &rotations; }

private:
    float duration;

    QString name;

    int id;

    unsigned int bone_count;

    QMap<int, QVector<QPair<float, QVector3D>>>   positions;
    QMap<int, QVector<QPair<float, QVector3D>>>   scalings;
    QMap<int, QVector<QPair<float, QQuaternion>>> rotations;

    QVector<int> boneRegister;
};

class Animations
{
public:
    Animations();
    ~Animations();

    void add(Animation* _animation);

    void setBones(Bones* _bones) { bones = _bones; }

    Animation* get(const int id) const { return animations[id]; }
    Bones*     getBones() const        { return bones; }

    const QVector<QString> getAnimationNames() const;
    const QVector<int>     getAnimationsIds() const;

    const int getAnimationCount() const { return animations.size(); }

private:
    Bones* bones;

    QMap<int, Animation*> animations;
};

class AnimationState
{
public:
    explicit AnimationState(Animations* anims);
    ~AnimationState();

    void update(const float frame_time);

    void setAnimation(const int id, const float animationTime = 0.0f);

    const int   getAnimation() const     { return current_animation; }
    const float getAnimationTime() const { return animation_time; }

    const QVector<QMatrix4x4> getTransforms() const;

    Animations* getAnimations() const { return animations; }

private:
    Animations* animations;

    int current_animation;

    float animation_time;

    QMap<int, QPair<float, int>> position_state;
    QMap<int, QPair<float, int>> rotation_state;
    QMap<int, QPair<float, int>> scaling_state;

    QMap<int, QMatrix4x4> transforms;
};

#endif // ANIMATION_H
