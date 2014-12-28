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

#include "animation.h"

Animation::Animation(const QString animation_name, const float animation_duration, const int animation_id)
{
    name     = animation_name;
    duration = animation_duration;
    id       = animation_id;
}

Animation::~Animation()
{
}

const QVector<QMatrix4x4> Animation::getTransforms(const float time_stamp, QMap<int, QPair<float, int>>* position_state, QMap<int, QPair<float, int>>* scaling_state, QMap<int, QPair<float, int>>* rotation_state) const
{
    QVector<QMatrix4x4> out;

    for(uint bone_index = 0; bone_index < bone_count; ++bone_index)
    {
        QVector3D position;
        QVector3D scaling;

        QQuaternion rotation;

        int i = (*position_state)[bone_index].second;

        if((*position_state)[bone_index].first > time_stamp)
            i = 0;

        for(i; i <= positions[bone_index].size(); ++i)
        {
            if(time_stamp < positions[bone_index][i].first)
            {
                if(i == 0)
                    position = positions[bone_index][i].second;
                else
                {
                    float diff      = positions[bone_index][i].first - positions[bone_index][i - 1].first;
                    float factor    = (time_stamp - positions[bone_index][i - 1].first) / diff;
                    float newfactor = (factor < 0.0f) ? 0.0f : (factor > 1.0f) ? 1.0f : factor;

                    position = positions[bone_index][i - 1].second * (1.0 - newfactor) + positions[bone_index][i].second * newfactor;
                }

                break;
            }
        }

        (*position_state)[bone_index] = QPair<float, int>(time_stamp, i);

        i = (*scaling_state)[bone_index].second;

        if((*scaling_state)[bone_index].first > time_stamp)
            i = 0;

        for(i; i < scalings[bone_index].size(); ++i)
        {
            if(time_stamp < scalings[bone_index][i].first)
            {
                if(i == 0)
                    scaling = scalings[bone_index][i].second;
                else
                {
                    float diff      = scalings[bone_index][i].first - scalings[bone_index][i - 1].first;
                    float factor    = (time_stamp - scalings[bone_index][i - 1].first) / diff;
                    float newfactor = (factor < 0.0f) ? 0.0f : (factor > 1.0f) ? 1.0f : factor;

                    scaling = scalings[bone_index][i - 1].second * (1.0 - newfactor) + scalings[bone_index][i].second * newfactor;
                }

                break;
            }
        }

        (*scaling_state)[bone_index] = QPair<float, unsigned int>(time_stamp, i);

        i = (*rotation_state)[bone_index].second;

        if((*rotation_state)[bone_index].first > time_stamp)
            i = 0;

        for(i; i < rotations[bone_index].size(); ++i)
        {
            if(time_stamp < rotations[bone_index][i].first)
            {
                rotation = rotations[bone_index][i].second;

                break;
            }
        }

        (*rotation_state)[bone_index] = QPair<float, int>(time_stamp, i);

        QMatrix4x4 mat;

        if(!scaling.isNull())
            mat.scale(scaling);

        if(!position.isNull())
            mat.translate(position);

        if(!rotation.isNull())
            mat.rotate(rotation);

        out.push_back(mat);
    }

    return out;
}

void Animation::addBonePosition(const int bone_id, const float time, const QVector3D position)
{
    positions[bone_id].push_back(QPair<float, QVector3D>(time, position));
}

void Animation::addBoneScaling(const int bone_id, const float time, const QVector3D scaling)
{
    scalings[bone_id].push_back(QPair<float, QVector3D>(time, scaling));
}

void Animation::addBoneRotation(const int bone_id, const float time, const QQuaternion rotation)
{
    rotations[bone_id].push_back(QPair<float, QQuaternion>(time, rotation));
}

void Animation::registerBone(const int boneId)
{
    boneRegister.push_back(boneId);
}

/// Animations
Animations::Animations()
{
}

Animations::~Animations()
{
    QMap<QString, Animation*> animations;

    QList<Animation*> values = animations.values();

    for(int i = 0; i < values.size(); ++i)
    {
        Animation* animation = values.at(i);

        delete animation;

        animation = 0;
    }
}

void Animations::add(Animation* _animation)
{
    animations[_animation->getId()] = _animation;
}

const QVector<QString> Animations::getAnimationNames() const
{
    QVector<QString> names;

    QList<Animation*> stack = animations.values();

    for(int i = 0; i < stack.size(); ++i)
        names.push_back(stack.at(i)->getName());

    return names;
}

const QVector<int> Animations::getAnimationsIds() const
{
    return animations.keys().toVector();
}

/// AnimationState
AnimationState::AnimationState(Animations* anims)
{
    current_animation = 0;
    animations        = anims;
    animation_time    = 0.0f;

    int bone_count = animations->getBones()->getBoneCount();

    for(int i = 0; i < bone_count; ++i)
    {
        position_state[i] = QPair<float, int>(0.0f, 0);
        scaling_state[i]  = QPair<float, int>(0.0f, 0);
        rotation_state[i] = QPair<float, int>(0.0f, 0);
    }

    update(0.0f);
}

AnimationState::~AnimationState()
{
}

void AnimationState::update(const float frame_time)
{
    Animation* animation = animations->get(current_animation);

    float animation_duration = animation->getDuration();

    animation_time += frame_time;

    if(animation_time > animation_duration - 1)
        animation_time = animation_time - animation_duration * float(int(animation_time / animation_duration));

    QVector<QMatrix4x4> unmultiplied_transforms = animation->getTransforms(animation_time, &position_state, &scaling_state, &rotation_state);

    for(int i = 0; i < unmultiplied_transforms.size(); ++i)
    {
        Bone* b = animations->getBones()->getBone(i);

        QMatrix4x4 offset = b->getOffset();
        QMatrix4x4 result = QMatrix4x4();

        while(b)
        {
            if(animation->isRegistered(i))
                result = unmultiplied_transforms[b->getId()] * result;
            else
                result = b->getNodeTransformation() * result;

            b = b->getParent();
        }

        transforms[i] =  result * offset;
    }
}

void AnimationState::setAnimation(const int id, const float animationTime)
{
    current_animation = id;
    animation_time    = animationTime;
}

const QVector<QMatrix4x4> AnimationState::getTransforms() const
{
    QVector<QMatrix4x4> out;

    for(int i = 0; i < transforms.size(); ++i)
        out.push_back(transforms[i]);

    return out;
}
