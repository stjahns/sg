#include <stdio.h>
#include <stdio.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"

#include "Skeleton.h"
#include "AssimpSkeletonLoader.h"
#include "TestHelpers.h"

using namespace glm;

TEST(ConvertToMat4, Identity)
{
	aiMatrix4x4 in;
	mat4 out = ConvertToMat4(in);
	EXPECT_MAT4_EQ(out, mat4());
}

TEST(ConvertToMat4, RotationZ)
{
	aiMatrix4x4 in;
	in.FromEulerAnglesXYZ(0, 0, 3);
	mat4 out = ConvertToMat4(in);

	mat4 expected = glm::rotate(mat4(), 3.0f, vec3(0.0f, 0.0f, 1.0f));

	EXPECT_MAT4_EQ(out, expected);
}

TEST(ConvertToMat4, RotationX)
{
	aiMatrix4x4 in;
	in.FromEulerAnglesXYZ(1, 0, 0);
	mat4 out = ConvertToMat4(in);

	mat4 expected = glm::rotate(mat4(), 1.0f, vec3(1.0f, 0.0f, 0.0f));

	EXPECT_MAT4_EQ(out, expected);
}

TEST(ConvertToMat4, Translation)
{
	aiMatrix4x4 in;
	in = aiMatrix4x4::Translation(aiVector3D(1, 2, 3), in);

	mat4 out = ConvertToMat4(in);

	mat4 expected = glm::translate(mat4(), vec3(1.0f, 2.0f, 3.0f));

	EXPECT_MAT4_EQ(out, expected);
}

TEST(Skeleton, FindBoneIndex_Found)
{
	Skeleton skeleton;

	skeleton.bones.emplace_back("1");
	skeleton.bones.emplace_back("2");
	skeleton.bones.emplace_back("3");
	skeleton.bones.emplace_back("4");

	BoneIndex index = skeleton.FindBoneIndex("3");

	EXPECT_EQ(index, BoneIndex(2));
}

TEST(Skeleton, FindBoneIndex_NotFound)
{
	Skeleton skeleton;

	skeleton.bones.emplace_back("1");
	skeleton.bones.emplace_back("2");
	skeleton.bones.emplace_back("3");
	skeleton.bones.emplace_back("4");

	BoneIndex index = skeleton.FindBoneIndex("5");

	EXPECT_EQ(index, BoneIndex::Invalid);
}

TEST(LoadSkeleton_Simple, ReturnsTrueOnSuccess)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf"), 0);

	ASSERT_NE(scene, nullptr);

	Skeleton skeleton;

	bool result = LoadSkeleton(*scene, skeleton);

	ASSERT_TRUE(result);
}

TEST(LoadSkeleton_Simple, AddsBones)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf"), 0);
	ASSERT_NE(scene, nullptr);

	Skeleton skeleton;

	bool result = LoadSkeleton(*scene, skeleton);

	ASSERT_EQ(skeleton.bones.size(), 2);
}

TEST(LoadSkeleton_Simple, SetsBoneParentIndices)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf"), 0);
	ASSERT_NE(scene, nullptr);

	Skeleton skeleton;

	bool result = LoadSkeleton(*scene, skeleton);

	ASSERT_EQ(skeleton.bones.size(), 2);
	EXPECT_EQ(skeleton.bones[0].parent, BoneIndex::Invalid);
	EXPECT_EQ(skeleton.bones[1].parent, BoneIndex(0));
}

TEST(LoadSkeleton_Simple, SetsBoneIds)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf"), 0);
	ASSERT_NE(scene, nullptr);

	Skeleton skeleton;

	bool result = LoadSkeleton(*scene, skeleton);

	ASSERT_EQ(skeleton.bones.size(), 2);
	EXPECT_EQ(skeleton.bones[0].id, "Bone");
	EXPECT_EQ(skeleton.bones[1].id, "Bone.001");
}

void BuildTwoBoneTestScene(aiScene& scene)
{
	scene.mNumMeshes = 1;
	scene.mMeshes = new aiMesh * [1];

	aiMesh* mesh = new aiMesh();
	scene.mMeshes[0] = mesh;

	scene.mRootNode = new aiNode("root");

	aiNode* parent = new aiNode("Parent");
	aiNode* child = new aiNode("Child");

	parent->mTransformation = aiMatrix4x4::Translation(aiVector3D(1, 2, 3), parent->mTransformation);
	child->mTransformation = aiMatrix4x4::Translation(aiVector3D(4, 5, 6), child->mTransformation);

	aiNode* rootChildren[1] = { parent };
	aiNode* parentChildren[1] = { child };
	
	scene.mRootNode->addChildren(1, rootChildren);
	parent->addChildren(1, parentChildren);

	mesh->mNumBones = 2;
	mesh->mBones = new aiBone * [2];
	mesh->mBones[0] = new aiBone();
	mesh->mBones[1] = new aiBone();
	mesh->mBones[0]->mName = "Parent";
	mesh->mBones[1]->mName = "Child";
	mesh->mBones[0]->mOffsetMatrix.FromEulerAnglesXYZ(1, 0, 0);
	mesh->mBones[1]->mOffsetMatrix.FromEulerAnglesXYZ(0, 1, 0);
}

TEST(LoadSkeleton_Simple, SetsInverseBindPoses)
{
	aiScene scene;
	BuildTwoBoneTestScene(scene);

	Skeleton skeleton;
	bool result = LoadSkeleton(scene, skeleton);

	mat4 expectedParent = glm::rotate(mat4(), 1.0f, vec3(1.0f, 0.0f, 0.0f));
	mat4 expectedChild = glm::rotate(mat4(), 1.0f, vec3(0.0f, 1.0f, 0.0f));

	ASSERT_EQ(skeleton.bones.size(), 2);
	EXPECT_MAT4_EQ(skeleton.bones[0].inverseBindPose, expectedParent);
	EXPECT_MAT4_EQ(skeleton.bones[1].inverseBindPose, expectedChild);
}

TEST(LoadSkeleton_Simple, SetsBindPose)
{
	aiScene scene;
	BuildTwoBoneTestScene(scene);

	Skeleton skeleton;
	bool result = LoadSkeleton(scene, skeleton);

	mat4 expectedParent = glm::translate(mat4(), vec3(1.0f, 2.0f, 3.0f));
	mat4 expectedChild = glm::translate(mat4(), vec3(4.0f, 5.0f, 6.0f));

	ASSERT_EQ(skeleton.bindPose.localTransforms.size(), 2);
	EXPECT_MAT4_EQ(skeleton.bindPose.localTransforms[0], expectedParent);
	EXPECT_MAT4_EQ(skeleton.bindPose.localTransforms[1], expectedChild);
}