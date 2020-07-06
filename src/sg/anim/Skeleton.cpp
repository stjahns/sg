#include <vector>

#include "Skeleton.h"

const BoneIndex BoneIndex::Invalid = BoneIndex(-1);

BoneIndex Skeleton::FindBoneIndex(std::string id) const
{
    for (int i = 0; i < bones.size(); ++i)
    {
        if (bones[i].id == id)
        {
            return i;
        }
    }

    return -1;
}

void Pose::ComputeObjectFromLocal(const Skeleton& skeleton)
{
    objectTransforms.resize(localTransforms.size());

    mat4 identity;

    for (int boneIndex = 0; boneIndex < skeleton.bones.size(); ++boneIndex)
    {
        BoneIndex parentIndex = skeleton.bones[boneIndex].parent;
        const mat4& parentTransform = parentIndex.IsValid() ? objectTransforms[parentIndex] : identity;
        const mat4& localTransform = localTransforms[boneIndex];

        objectTransforms[boneIndex] = parentTransform * localTransform;
    }
}
