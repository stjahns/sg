#include <stdio.h>
#include <stdio.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"

#include "Skeleton.h"
#include "AssimpSkeletonLoader.h"
#include "TestHelpers.h"

using namespace glm;

void LoadSkeletons(std::string path, Skeleton& assimp, Skeleton& gltf)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, 0);

	ASSERT_NE(scene, nullptr);

    ASSERT_TRUE(LoadSkeleton(*scene, assimp));

    tinygltf::TinyGLTF loader;
    tinygltf::Model gltfModel;
    std::string err;
    std::string warn;

    if (loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path))
    {
        gltf.Load(gltfModel);
    }
}

TEST(AssimpVsGLTF, Simple_BoneIdsMatch)
{
	std::string path = GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf");

	Skeleton assimpSkeleton;
	Skeleton gltfSkeleton;

    LoadSkeletons(path, assimpSkeleton, gltfSkeleton);

    ASSERT_EQ(gltfSkeleton.bones.size(), assimpSkeleton.bones.size());

    for (int i = 0; i < gltfSkeleton.bones.size(); ++i)
    {
        EXPECT_EQ(gltfSkeleton.bones[i].id, assimpSkeleton.bones[i].id);
    }
}

TEST(AssimpVsGLTF, Simple_InverseBindPosesMatch)
{
	std::string path = GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf");

	Skeleton assimpSkeleton;
	Skeleton gltfSkeleton;

    LoadSkeletons(path, assimpSkeleton, gltfSkeleton);

    ASSERT_EQ(gltfSkeleton.bones.size(), assimpSkeleton.bones.size());

    for (int i = 0; i < gltfSkeleton.bones.size(); ++i)
    {
        EXPECT_MAT4_EQ(gltfSkeleton.bones[i].inverseBindPose, assimpSkeleton.bones[i].inverseBindPose);
    }
}

TEST(AssimpVsGLTF, Simple_ParentsMatch)
{
	std::string path = GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf");

	Skeleton assimpSkeleton;
	Skeleton gltfSkeleton;

    LoadSkeletons(path, assimpSkeleton, gltfSkeleton);

    ASSERT_EQ(gltfSkeleton.bones.size(), assimpSkeleton.bones.size());

    for (int i = 0; i < gltfSkeleton.bones.size(); ++i)
    {
        EXPECT_EQ(gltfSkeleton.bones[i].parent, assimpSkeleton.bones[i].parent);
    }
}

TEST(AssimpVsGLTF, Simple_BindPosesMatch)
{
	std::string path = GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf");

	Skeleton assimpSkeleton;
	Skeleton gltfSkeleton;

    LoadSkeletons(path, assimpSkeleton, gltfSkeleton);

    ASSERT_EQ(gltfSkeleton.bindPose.localTransforms.size(), assimpSkeleton.bindPose.localTransforms.size());

    for (int i = 0; i < gltfSkeleton.bones.size(); ++i)
    {
        EXPECT_MAT4_EQ(gltfSkeleton.bindPose.localTransforms[i], assimpSkeleton.bindPose.localTransforms[i]);
    }
}

TEST(AssimpVsGLTF, Fox_BoneIdsMatch)
{
	std::string path = GetAssetPath("Models/Fox/gLTF/Fox.gltf");

	Skeleton assimpSkeleton;
	Skeleton gltfSkeleton;

    LoadSkeletons(path, assimpSkeleton, gltfSkeleton);

    ASSERT_EQ(gltfSkeleton.bones.size(), assimpSkeleton.bones.size());

    for (int i = 0; i < gltfSkeleton.bones.size(); ++i)
    {
        EXPECT_EQ(gltfSkeleton.bones[i].id, assimpSkeleton.bones[i].id);
    }
}

TEST(AssimpVsGLTF, Fox_InverseBindPosesMatch)
{
	std::string path = GetAssetPath("Models/Fox/gLTF/Fox.gltf");

	Skeleton assimpSkeleton;
	Skeleton gltfSkeleton;

    LoadSkeletons(path, assimpSkeleton, gltfSkeleton);

    ASSERT_EQ(gltfSkeleton.bones.size(), assimpSkeleton.bones.size());

    for (int i = 0; i < gltfSkeleton.bones.size(); ++i)
    {
        EXPECT_MAT4_NEAR(gltfSkeleton.bones[i].inverseBindPose, assimpSkeleton.bones[i].inverseBindPose, 0.001f);
    }
}

TEST(AssimpVsGLTF, Fox_ParentsMatch)
{
	std::string path = GetAssetPath("Models/Fox/gLTF/Fox.gltf");

	Skeleton assimpSkeleton;
	Skeleton gltfSkeleton;

    LoadSkeletons(path, assimpSkeleton, gltfSkeleton);

    ASSERT_EQ(gltfSkeleton.bones.size(), assimpSkeleton.bones.size());

    for (int i = 0; i < gltfSkeleton.bones.size(); ++i)
    {
        EXPECT_EQ(gltfSkeleton.bones[i].parent, assimpSkeleton.bones[i].parent);
    }
}

TEST(AssimpVsGLTF, Fox_BindPosesMatch)
{
	std::string path = GetAssetPath("Models/Fox/gLTF/Fox.gltf");

	Skeleton assimpSkeleton;
	Skeleton gltfSkeleton;

    LoadSkeletons(path, assimpSkeleton, gltfSkeleton);

    ASSERT_EQ(gltfSkeleton.bindPose.localTransforms.size(), assimpSkeleton.bindPose.localTransforms.size());

    for (int i = 0; i < gltfSkeleton.bones.size(); ++i)
    {
        EXPECT_MAT4_EQ(gltfSkeleton.bindPose.localTransforms[i], assimpSkeleton.bindPose.localTransforms[i]);
    }
}

