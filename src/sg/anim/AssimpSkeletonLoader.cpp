#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <string>
#include <vector>

#include "Skeleton.h"

using namespace glm;

mat4 ConvertToMat4(aiMatrix4x4 in)
{
	mat4 out;

	out[0][0] = in.a1;
	out[1][0] = in.a2;
	out[2][0] = in.a3;
	out[3][0] = in.a4;

	out[0][1] = in.b1;
	out[1][1] = in.b2;
	out[2][1] = in.b3;
	out[3][1] = in.b4;

	out[0][2] = in.c1;
	out[1][2] = in.c2;
	out[2][2] = in.c3;
	out[3][2] = in.c4;

	out[0][3] = in.d1;
	out[1][3] = in.d2;
	out[2][3] = in.d3;
	out[3][3] = in.d4;

	return out;
}

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

		bone.inverseBindPose = ConvertToMat4(sourceBone.mOffsetMatrix);

		skeleton.bindPose.localTransforms.push_back(ConvertToMat4(node->mTransformation));
		skeleton.bones.push_back(bone);
	}

	skeleton.bindPose.ComputeObjectFromLocal(skeleton);

	return true;
}
