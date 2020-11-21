#include <stdio.h>
#include <stdio.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"

#include "Skeleton.h"
#include "AssimpSkeletonLoader.h"
#include "AssimpClipLoader.h"
#include "AnimationClip.h"
#include "TestHelpers.h"

using namespace glm;

// TODO - what about times outside key range?

TEST(LoadClip, ReturnsTrue)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf"), 0);

	ASSERT_NE(scene, nullptr);
	ASSERT_TRUE(scene->HasAnimations() && scene->mNumAnimations > 0);

	AnimationClip clip;
	Skeleton skeleton;

	bool result = LoadClip(*scene, skeleton, clip);

	EXPECT_EQ(result, true);
}

TEST(LoadClip, LoadsDuration)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf"), 0);

	ASSERT_NE(scene, nullptr);
	ASSERT_TRUE(scene->HasAnimations() && scene->mNumAnimations > 0);

	AnimationClip clip;
	Skeleton skeleton;

	LoadSkeleton(*scene, skeleton);

	bool result = LoadClip(*scene, skeleton, clip);

	EXPECT_FLOAT_EQ(clip.duration, scene->mAnimations[0]->mDuration / 1000.0f);
}

TEST(LoadClip, LoadsPositionChannel)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf"), 0);

	ASSERT_NE(scene, nullptr);
	ASSERT_TRUE(scene->HasAnimations() && scene->mNumAnimations > 0);

	AnimationClip clip;
	Skeleton skeleton;

	LoadSkeleton(*scene, skeleton);

	bool result = LoadClip(*scene, skeleton, clip);

	ASSERT_EQ(clip.translationChannels.size(), 1);
	EXPECT_TRUE(clip.translationChannels[0].GetTarget().IsValid());
}

TEST(LoadClip, LoadsRotationChannel)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf"), 0);

	ASSERT_NE(scene, nullptr);
	ASSERT_TRUE(scene->HasAnimations() && scene->mNumAnimations > 0);

	AnimationClip clip;
	Skeleton skeleton;

	LoadSkeleton(*scene, skeleton);

	bool result = LoadClip(*scene, skeleton, clip);

	ASSERT_EQ(clip.rotationChannels.size(), 1);
	EXPECT_TRUE(clip.rotationChannels[0].GetTarget().IsValid());
}

