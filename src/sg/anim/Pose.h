#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <glm/gtc/quaternion.hpp>

#include "BoneIndex.h"

using namespace glm;

class Skeleton;

class Pose
{
public:

    void ComputeObjectFromLocal(const Skeleton& skeleton);

    // TODO -- clean up this interface...

    const mat4x4& GetLocalTransform(BoneIndex index) const { return localTransforms[index]; }
    mat4x4& GetLocalTransform(BoneIndex index) { return localTransforms[index]; }

    const mat4x4& GetObjectTransform(BoneIndex index) const { return objectTransforms[index]; }
    mat4x4& GetObjectTransform(BoneIndex index) { return objectTransforms[index]; }

    std::vector<mat4x4> localTransforms;

    std::vector<mat4x4> objectTransforms;

};

struct Transform
{
    vec3 translation;
    quat rotation;

    Transform()
    {
    }

    Transform(const vec3& v, const quat& q) : translation(v), rotation(q)
    {
    }
};

class AnimationPose
{
    std::vector<Transform> transforms;
    std::vector<bool> valid;

    void SetValid(BoneIndex index, bool value)
    {
        if (index >= valid.size())
        {
            valid.resize(index + 1, false);
        }

        valid[index] = value;
    }

public:

    int GetBoneCount() const { return transforms.size(); }

    bool GetTranslation(BoneIndex index, vec3& out) const
    { 
        if (index >= valid.size())
        {
            return false;
        }

        out = transforms[index].translation;
        return valid[index];
    }

    bool GetRotation(BoneIndex index, quat& out) const
    { 
        if (index >= valid.size())
        {
            return false;
        }

        out = transforms[index].rotation;
        return valid[index];
    }

    void SetTranslation(BoneIndex index, vec3 value)
    {
        if (index >= transforms.size())
        {
            transforms.resize(index + 1);
        }

        transforms[index].translation = value;
        SetValid(index, true);
    }

    void SetRotation(BoneIndex index, quat value)
    {
        if (index >= transforms.size())
        {
            transforms.resize(index + 1);
        }

        transforms[index].rotation = value;
        SetValid(index, true);
    }
};
