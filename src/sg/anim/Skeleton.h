#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "BoneIndex.h"
#include "Pose.h"

using namespace glm;

class Bone
{
public:

    Bone(std::string id) : parent(BoneIndex::Invalid) , id(id)
    {
    }

    Bone(std::string id, BoneIndex parent) : parent(parent), id(id)
    {
    }

    std::string id;
    BoneIndex parent;
    glm::mat4x4 inverseBindPose;
};

namespace tinygltf
{
    class Model;
}

class Skeleton
{
    public:

        std::vector<Bone> bones;

        BoneIndex FindBoneIndex(std::string id) const;

        bool Load(const tinygltf::Model& gltfSource);

        void ApplyAnimationPose(const AnimationPose& animPose, Pose& skeletonPose);

        Pose currentPose;
        Pose bindPose;

    private:

};