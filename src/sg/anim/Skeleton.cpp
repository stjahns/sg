#include <vector>

#include "Skeleton.h"

const BoneIndex BoneIndex::Invalid = BoneIndex(-1);

BoneIndex Skeleton::FindBoneIndex(std::string id)
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
