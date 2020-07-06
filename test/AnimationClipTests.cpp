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

TEST(PositionChannelTests, NoKeys_DefaultValue)
{
	TranslationChannel channel;

	vec3 v = channel.Evaulate(1.0f);

	EXPECT_VEC3_EQ(v, vec3());
}

TEST(PositionChannelTests, OneKey)
{
	TranslationChannel channel;

	vec3 expected(1.0f, 2.0f, 3.0f);

	channel.keys.emplace_back(0.0f, expected);

	vec3 v = channel.Evaulate(1.0f);

	EXPECT_VEC3_EQ(v, expected);
}

TEST(PositionChannelTests, TwoKeys)
{
	TranslationChannel channel;

	channel.keys.emplace_back(0.0f, vec3(0));
	channel.keys.emplace_back(1.0f, vec3(1));

	EXPECT_VEC3_EQ(channel.Evaulate(0.0f), vec3(0));
	EXPECT_VEC3_EQ(channel.Evaulate(1.0f), vec3(1));
}

TEST(PositionChannelTests, TwoKeys_Interpolates)
{
	TranslationChannel channel;

	channel.keys.emplace_back(0.0f, vec3(0));
	channel.keys.emplace_back(1.0f, vec3(1));

	EXPECT_VEC3_EQ(channel.Evaulate(0.5f), vec3(0.5));
}

TEST(RotationChannelTests, NoKeys_DefaultValue)
{
	TranslationChannel channel;

	quat q = channel.Evaulate(1.0f);

	EXPECT_QUAT_EQ(q, quat());
}

TEST(RotationChannelTests, OneKey)
{
	RotationChannel channel;

	quat expected(vec3(0.5f, 0.5f, 0.5f));

	channel.keys.emplace_back(0.0f, expected);

	quat q = channel.Evaulate(1.0f);

	EXPECT_QUAT_EQ(q, expected);
}

TEST(RotationChannelTests, TwoKeys)
{
	RotationChannel channel;

	quat qStart = quat(vec3(0, 0, 0));
	quat qEnd = quat(vec3(1, 0, 0));

	channel.keys.emplace_back(0.0f, qStart);
	channel.keys.emplace_back(1.0f, qEnd);

	EXPECT_QUAT_EQ(channel.Evaulate(0.0f), qStart);
	EXPECT_QUAT_EQ(channel.Evaulate(1.0f), qEnd);
}

TEST(RotationChannelTests, TwoKeys_Interpolates)
{
	RotationChannel channel;

	quat qStart = quat(vec3(0, 0, 0));
	quat qEnd = quat(vec3(1, 0, 0));

	channel.keys.emplace_back(0.0f, qStart);
	channel.keys.emplace_back(1.0f, qEnd);

	EXPECT_QUAT_EQ(channel.Evaulate(0.5f), quat(vec3(0.5, 0, 0)));
}

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

	EXPECT_EQ(clip.duration, scene->mAnimations[0]->mDuration);
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

	EXPECT_EQ(clip.positionChannels.size(), 1);
	EXPECT_TRUE(clip.positionChannels[0].target.IsValid());
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

	EXPECT_EQ(clip.rotationChannels.size(), 1);
	EXPECT_TRUE(clip.rotationChannels[0].target.IsValid());
}

TEST(AnimationClip, EvaluateClip_WithPositionChannels)
{
    Skeleton skeleton;
    skeleton.bones.emplace_back("root");
    skeleton.bones.emplace_back("bone1", 0);
    skeleton.bones.emplace_back("bone2", 1);

	BoneIndex boneIndex(1);
	AnimationClip clip;

	TranslationChannel channel(boneIndex);
	channel.keys.emplace_back(0.0f, vec3(0));
	channel.keys.emplace_back(1.0f, vec3(1));
	clip.positionChannels.push_back(channel);

	Pose pose;
	pose.localTransforms.resize(3); // TODO -- something to copy/init a pose?

	clip.EvaulatePose(skeleton, 0.5f, pose);

	mat4 transform = pose.localTransforms[boneIndex];
	vec3 position = transform * vec4(0, 0, 0, 1);

	EXPECT_VEC3_EQ(position, vec3(0.5))
}

TEST(AnimationClip, EvaluateClip_WithRotationChannels)
{
    Skeleton skeleton;
    skeleton.bones.emplace_back("root");
    skeleton.bones.emplace_back("bone1", 0);
    skeleton.bones.emplace_back("bone2", 1);

	BoneIndex boneIndex(1);
	AnimationClip clip;

	RotationChannel channel(boneIndex);
	channel.keys.emplace_back(0.0f, quat(vec3(0, 0, 0)));
    channel.keys.emplace_back(1.0f, quat(vec3(half_pi<float>(), 0, 0)));
	clip.rotationChannels.push_back(channel);

	Pose pose;
	pose.localTransforms.resize(3); // TODO -- something to copy/init a pose?

	clip.EvaulatePose(skeleton, 0.5f, pose);

    quat rotation = quat_cast(pose.localTransforms[boneIndex]);

	EXPECT_QUAT_EQ(rotation, quat(vec3(quarter_pi<float>(), 0, 0)));
}

TEST(AnimationClip, EvaluateClip_WithTranslationAndRotation)
{
    Skeleton skeleton;
    skeleton.bones.emplace_back("root");
    skeleton.bones.emplace_back("bone1", 0);
    skeleton.bones.emplace_back("bone2", 1);

	BoneIndex boneIndex(1);
	AnimationClip clip;

	RotationChannel rotationChannel(boneIndex);
	rotationChannel.keys.emplace_back(0.0f, quat(vec3(0, 0, 0)));
    rotationChannel.keys.emplace_back(1.0f, quat(vec3(half_pi<float>(), 0, 0)));
	clip.rotationChannels.push_back(rotationChannel);

	TranslationChannel translationChannel(boneIndex);
	translationChannel.keys.emplace_back(0.0f, vec3(0));
	translationChannel.keys.emplace_back(1.0f, vec3(0, 1, 0));
	clip.positionChannels.push_back(translationChannel);

	Pose pose;
	pose.localTransforms.resize(3); // TODO -- something to copy/init a pose?

	clip.EvaulatePose(skeleton, 0.5f, pose);

	mat4 transform = pose.localTransforms[boneIndex];
    quat rotation = quat_cast(transform);
	vec3 position = transform * vec4(0, 0, 0, 1);

	EXPECT_QUAT_EQ(rotation, quat(vec3(quarter_pi<float>(), 0, 0)));
	EXPECT_VEC3_EQ(position, vec3(0, 0.5f, 0));
}
