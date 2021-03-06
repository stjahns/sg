#include <vector>
#include <tiny_gltf.h>

#include <glm/gtc/type_ptr.hpp>

#include "Skeleton.h"
#include "Pose.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

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

namespace
{
    void LoadInverseBindMatrices(const tinygltf::Model& gltfSource, tinygltf::Skin skin, std::vector<mat4>& inverseBindMatrices)
    {
        const tinygltf::Accessor& accessor = gltfSource.accessors[skin.inverseBindMatrices];
        const tinygltf::BufferView& bufferView = gltfSource.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = gltfSource.buffers[bufferView.buffer];

        inverseBindMatrices.resize(accessor.count);
        memcpy(inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
    }

    mat4 GetNodeLocalTransform(const tinygltf::Node& node)
    {
        mat4 matrix, translation, rotation;

        if (node.matrix.size() == 16)
        {
            matrix = glm::make_mat4(node.matrix.data());
        }

        if (node.translation.size() == 3)
        {
            vec3 t = glm::make_vec3(node.translation.data());
			translation = glm::translate(mat4(1.0f), t);
        }

		if (node.rotation.size() == 4)
		{
            glm::quat r = glm::make_quat(node.rotation.data());
            rotation = mat4(r);
		}

        return matrix * translation * rotation;
    }

    void WalkSceneGraph(const tinygltf::Model& gltfSource, int nodeIndex, int parentIndex, std::vector<int>& parentIndices, std::vector<mat4>& globalTransforms)
    {
        const tinygltf::Node node = gltfSource.nodes[nodeIndex];

        parentIndices[nodeIndex] = parentIndex;

        mat4 parentTransform;

        if (parentIndex >= 0)
        {
            parentTransform = globalTransforms[parentIndex];
        }

        globalTransforms[nodeIndex] = parentTransform * GetNodeLocalTransform(node);

        for (int child : node.children)
        {
            WalkSceneGraph(gltfSource, child, nodeIndex, parentIndices, globalTransforms);
        }
    }

    BoneIndex NodeToBoneIndex(tinygltf::Skin skin, int nodeIndex)
    {
        for (int i = 0; i < skin.joints.size(); ++i)
        {
            if (skin.joints[i] == nodeIndex)
            {
                return BoneIndex(i);
            }
        }

        return BoneIndex::Invalid;
    }
}

bool Skeleton::Load(const tinygltf::Model& gltfSource)
{
    if (gltfSource.skins.empty() || gltfSource.nodes.empty())
    {
        return false;
    }

    tinygltf::Skin skin = gltfSource.skins.front();

    std::vector<mat4> inverseBindMatrices;
    LoadInverseBindMatrices(gltfSource, skin, inverseBindMatrices);

    std::vector<int> parentIndices;
    std::vector<mat4> globalTransforms;
    parentIndices.resize(gltfSource.nodes.size());
    globalTransforms.resize(gltfSource.nodes.size());

    WalkSceneGraph(gltfSource, 0, -1, parentIndices, globalTransforms);

    for (int nodeIndex : skin.joints)
    {
        const tinygltf::Node& node = gltfSource.nodes[nodeIndex];

        BoneIndex boneIndex = bones.size();
        BoneIndex parentIndex = NodeToBoneIndex(skin, parentIndices[nodeIndex]);

        Bone bone(node.name, parentIndex);
        bone.inverseBindPose = inverseBindMatrices[boneIndex];

        mat4 nodeLocalTransform = GetNodeLocalTransform(node);
        bindPose.localTransforms.push_back(nodeLocalTransform);

        bones.push_back(bone);
    }

    currentPose.localTransforms.resize(bindPose.localTransforms.size());
    currentPose.objectTransforms.resize(bindPose.localTransforms.size());

    for (int i = 0; i < bindPose.localTransforms.size(); ++i)
    {
        currentPose.localTransforms[i] = bindPose.localTransforms[i];
    }
}

void Skeleton::ApplyAnimationPose(const AnimationPose& animPose, Pose& skeletonPose)
{
    for (int i = 0; i < skeletonPose.localTransforms.size(); ++i)
    {
        mat4 pose = skeletonPose.localTransforms[i];

        quat rotation;
        if (animPose.GetRotation(i, rotation))
        {
            vec4 originalTranslation = pose[3];
            pose = mat4_cast(rotation);
            pose[3] = originalTranslation;
        }

        vec3 translation;
        if (animPose.GetTranslation(i, translation))
        {
            pose[3] = vec4(translation, 1.0f);
        }

        skeletonPose.localTransforms[i] = pose;
    }
}
