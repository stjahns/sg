#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

using namespace glm;

class BoneIndex
{
public:

    BoneIndex() : index(Invalid.index)
    {
    }

    BoneIndex(int index) : index(index)
    {
    }

    bool IsValid() const { return index != Invalid.index; }

    operator int() const { return index; }

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

    Bone(std::string id, BoneIndex parent) : parent(parent), id(id)
    {
    }

    std::string id;
    BoneIndex parent;
    glm::mat4x4 inverseBindPose;
};

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

        Pose bindPose;

    private:

};