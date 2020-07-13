#pragma once

#include "assimp/matrix4x4.h"
#include "Skeleton.h"

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <string>
#include <vector>

glm::mat4 ConvertToMat4(aiMatrix4x4 in);

bool LoadSkeleton(const aiScene& scene, Skeleton& skeleton)
{
	const aiNode& root = *scene.mRootNode;
	const aiMesh& mesh = *scene.mMeshes[0]; // TODO -- handle multiple meshes

	for (int i = 0; i < mesh.mNumBones; ++i)
	{
		const aiBone& sourceBone = *mesh.mBones[i];
		const aiNode* node = root.FindNode(sourceBone.mName);

		if (!node)
		{
			// missing node for bone... TODO -- can we have some kind of error type?
            return false;
		}

		Bone bone(sourceBone.mName.C_Str());

        // This only works if bones are defined in the right order...
        // we might need to walk the node tree instead
		const aiNode* parent = node->mParent;
		if (parent)
		{
            bone.parent = skeleton.FindBoneIndex(parent->mName.C_Str());
		}

		mat4 bindPoseLocal = ConvertToMat4(node->mTransformation);

		skeleton.bindPose.localTransforms.push_back(bindPoseLocal);
		skeleton.bones.push_back(bone);
	}

	skeleton.bindPose.ComputeObjectFromLocal(skeleton);

	for (int i = 0; i < skeleton.bones.size(); ++i)
	{
		skeleton.bones[i].inverseBindPose = inverse(skeleton.bindPose.objectTransforms[i]);
	}

    skeleton.currentPose.localTransforms.resize(skeleton.bindPose.localTransforms.size());
    skeleton.currentPose.objectTransforms.resize(skeleton.bindPose.localTransforms.size());

    for (int i = 0; i < skeleton.bindPose.localTransforms.size(); ++i)
    {
        skeleton.currentPose.localTransforms[i] = skeleton.bindPose.localTransforms[i];
    }

	return true;
}
