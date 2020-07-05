#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

class BoneIndex
{
public:

    BoneIndex(int index) : index(index)
    {
    }

    bool IsValid() { return index == Invalid.index; }

    explicit operator int() const { return index; }

    bool operator== (const BoneIndex& other) const { return index == other.index; }

    static const BoneIndex Invalid;

private:

    int index;
};

class Bone
{
public:

    Bone(std::string id) : parent(BoneIndex::Invalid) , id(id)
    {
    }

    std::string id;
    BoneIndex parent;
    glm::mat4x4 inverseBindPose;
};

class Pose
{
public:
    std::vector<glm::mat4x4> localTransforms;
};

class Skeleton
{
    public:

        std::vector<Bone> bones;

        BoneIndex FindBoneIndex(std::string id);

        Pose bindPose;

    private:

};