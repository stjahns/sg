#include <vector>
#include <tiny_gltf.h>

#include <glm/gtc/type_ptr.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include "Skeleton.h"
#include "Pose.h"

void Pose::ComputeObjectFromLocal(const Skeleton& skeleton)
{
    objectTransforms.resize(localTransforms.size());

    mat4 identity(1.0f);

    for (int boneIndex = 0; boneIndex < skeleton.bones.size(); ++boneIndex)
    {
        BoneIndex parentIndex = skeleton.bones[boneIndex].parent;
        const mat4& parentTransform = parentIndex.IsValid() ? objectTransforms[parentIndex] : identity;
        const mat4& localTransform = localTransforms[boneIndex];

        objectTransforms[boneIndex] = parentTransform * localTransform;
    }
}
