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

#include "ianimation.h"

IAnimation::IAnimation(QString animation_name, float _duration, int id)
{
    name = animation_name;
    duration = _duration;
    this->id = id;
}

IAnimation::~IAnimation()
{
}

int IAnimation::getId()
{
    return id;
}

float IAnimation::getDuration()
{
    return duration;
}

QString IAnimation::getName()
{
    return name;
}

QVector<QMatrix4x4> IAnimation::getTransforms(float time_stamp, QMap<int, QPair<float, int> > *position_state, QMap<int, QPair<float, int> > *scaling_state, QMap<int, QPair<float, int> > *rotation_state)
{
    QVector<QMatrix4x4> out;
    for (uint bone_index = 0; bone_index < bone_count; bone_index++)
    {
        QVector3D position;
        QVector3D scaling;
        QQuaternion rotation;

        int i = (*position_state)[bone_index].second;
        if ((*position_state)[bone_index].first > time_stamp) {
            i = 0;
        }
        for (i; i <= positions[bone_index].size(); i++) {
            if (time_stamp < positions[bone_index][i].first) {
                if (i == 0) {
                    position = positions[bone_index][i].second;
                } else {
                    float diff = positions[bone_index][i].first - positions[bone_index][i - 1].first;
                    float factor = (time_stamp - positions[bone_index][i - 1].first) / diff;
                    float newfactor = (factor < 0.0f) ? 0.0f : (factor > 1.0f) ? 1.0f : factor;
                    position = positions[bone_index][i - 1].second * (1.0 - newfactor) + positions[bone_index][i].second * newfactor;
                }
                break;
            }
        }
        (*position_state)[bone_index] = QPair<float, int>(time_stamp, i);

        i = (*scaling_state)[bone_index].second;
        if ((*scaling_state)[bone_index].first > time_stamp) {
            i = 0;
        }
        for (i; i < scalings[bone_index].size(); i++) {
            if (time_stamp < scalings[bone_index][i].first) {
                if (i == 0) {
                    scaling = scalings[bone_index][i].second;
                } else {
                    float diff = scalings[bone_index][i].first - scalings[bone_index][i - 1].first;
                    float factor = (time_stamp - scalings[bone_index][i - 1].first) / diff;
                    float newfactor = (factor < 0.0f) ? 0.0f : (factor > 1.0f) ? 1.0f : factor;
                    scaling = scalings[bone_index][i - 1].second * (1.0 - newfactor) + scalings[bone_index][i].second * newfactor;
                }
                break;
            }
        }
        (*scaling_state)[bone_index] = QPair<float, unsigned int>(time_stamp, i);

        i = (*rotation_state)[bone_index].second;
        if ((*rotation_state)[bone_index].first > time_stamp) {
            i = 0;
        }
        for (i; i < rotations[bone_index].size(); i++) {
            if (time_stamp < rotations[bone_index][i].first) {
                rotation = rotations[bone_index][i].second;
                break;
            }
        }
        (*rotation_state)[bone_index] = QPair<float, int>(time_stamp, i);

        QMatrix4x4 mat;
        if (!scaling.isNull())
            mat.scale(scaling);
        if (!position.isNull())
            mat.translate(position);
        if (!rotation.isNull())
            mat.rotate(rotation);
        out.push_back(mat);
    }
    return out;
}

int IAnimation::getBoneCount()
{
    return bone_count;
}

void IAnimation::setBoneCount(int bone_count)
{
    this->bone_count = bone_count;
}

void IAnimation::addBonePosition(int bone_id, float time, QVector3D position)
{
    positions[bone_id].push_back(QPair<float, QVector3D>(time, position));
}

void IAnimation::addBoneScaling(int bone_id, float time, QVector3D scaling)
{
    scalings[bone_id].push_back(QPair<float, QVector3D>(time, scaling));
}

void IAnimation::addBoneRotation(int bone_id, float time, QQuaternion rotation)
{
    rotations[bone_id].push_back(QPair<float, QQuaternion>(time, rotation));
}

QMap<int, QVector<QPair<float, QVector3D> > > *IAnimation::getPositions()
{
    return &positions;
}

QMap<int, QVector<QPair<float, QVector3D> > > *IAnimation::getScalings()
{
    return &scalings;
}

QMap<int, QVector<QPair<float, QQuaternion> > > *IAnimation::getRotations()
{
    return &rotations;
}

void IAnimation::registerBone(int boneId)
{
    boneRegister.push_back(boneId);
}

bool IAnimation::isRegistered(int boneId)
{
    return boneRegister.contains(boneId);
}

IAnimations::IAnimations()
{
}

IAnimations::~IAnimations()
{
    QMap<QString, IAnimation *> animations;

    QList<IAnimation *> values = animations.values();
    for(int i = 0; i < values.size(); ++i)
    {
        IAnimation *animation = values.at(i);
        delete animation;
        animation = 0;
    }
}

void IAnimations::add(IAnimation *_animation)
{
    animations[_animation->getId()] = _animation;
}

IAnimation *IAnimations::get(int id)
{
    return animations[id];
}

void IAnimations::setBones(IBones *Bones)
{
    this->Bones = Bones;
}

IBones *IAnimations::getBones()
{
    return Bones;
}

QVector<QString> IAnimations::getAnimationNames()
{
    QVector<QString> names;

    QList<IAnimation *> stack = animations.values();
    for(int i = 0; i < stack.size(); ++i)
    {
        names.push_back(stack.at(i)->getName());
    }
    return names;
}

QVector<int> IAnimations::getAnimationsIds()
{
    return animations.keys().toVector();
}

int IAnimations::getAnimationCount()
{
    return animations.size();
}


IAnimationState::IAnimationState(IAnimations *animations)
{
    current_animation = 0;
    this->animations = animations;
    animation_time = 0.0f;
    int bone_count = animations->getBones()->getBoneCount();
    for (int i = 0; i < bone_count; i++) {
        position_state[i] = QPair<float, int>(0.0f, 0);
        scaling_state[i] = QPair<float, int>(0.0f, 0);
        rotation_state[i] = QPair<float, int>(0.0f, 0);
    }
    update(0.0f);
}

IAnimationState::~IAnimationState()
{

}

void IAnimationState::setAnimation(int id, float animationTime)
{
    current_animation = id;
    animation_time = animationTime;
}

int IAnimationState::getAnimation()
{
    return current_animation;
}

float IAnimationState::getAnimationTime()
{
    return animation_time;
}

IAnimations *IAnimationState::getAnimations()
{
    return animations;
}

void IAnimationState::update(float frame_time)
{
    IAnimation *animation = animations->get(current_animation);
    float animation_duration = animation->getDuration();
    animation_time += frame_time;
    if (animation_time > animation_duration-1) {
        animation_time = animation_time - animation_duration * float(int(animation_time / animation_duration));
    }

    QVector<QMatrix4x4> unmultiplied_transforms = animation->getTransforms(animation_time, &position_state, &scaling_state, &rotation_state);
    for (int i = 0; i < unmultiplied_transforms.size(); i++) {
        IBone *b = animations->getBones()->getBone(i);
        QMatrix4x4 offset = b->getOffset();
        QMatrix4x4 result = QMatrix4x4();
        while (b) {
            if(animation->isRegistered(i))
                result = unmultiplied_transforms[b->getId()] * result;
            else
                result = b->getNodeTransformation() * result;
            b = b->getParent();
        }
        transforms[i] =  result * offset;
    }
}

QVector<QMatrix4x4> IAnimationState::getTransforms()
{
    QVector<QMatrix4x4> out;
    for (int i = 0; i < transforms.size(); i++) {
        out.push_back(transforms[i]);
    }
    return out;
}
