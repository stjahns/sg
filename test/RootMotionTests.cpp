#include <stdio.h>
#include <stdio.h>
#include <gtest/gtest.h>
// TODO #include <gmock/gmock.h>

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
#include "AnimationNode.h"
#include "TestHelpers.h"

using namespace glm;

void MakeClip(AnimationClip& clip)
{
    Skeleton skeleton;
    skeleton.bones.emplace_back("root");
    skeleton.bones.emplace_back("bone1", 0);
    skeleton.bones.emplace_back("bone2", 1);

	BoneIndex boneIndex(0);

	RotationChannel rotationChannel(boneIndex);
	rotationChannel.AddKey(0.0f, quat(vec3(0.0f, 0.0f, 0.0f)));
	rotationChannel.AddKey(1.0f, quat(vec3(0.1f, 0.0f, 0.0f)));
	rotationChannel.AddKey(2.0f, quat(vec3(0.2f, 0.0f, 0.0f)));
	rotationChannel.AddKey(3.0f, quat(vec3(0.3f, 0.0f, 0.0f)));

	clip.rotationChannels.push_back(rotationChannel);

	TranslationChannel translationChannel(boneIndex);
	translationChannel.AddKey(0.0f, vec3(0.0f));
	translationChannel.AddKey(1.0f, vec3(0.0f, 1.0f, 0.0f));
	translationChannel.AddKey(2.0f, vec3(0.0f, 2.0f, 0.0f));
	translationChannel.AddKey(3.0f, vec3(0.0f, 3.0f, 0.0f));
	clip.translationChannels.push_back(translationChannel);
}

TEST(RootMotionTests, CreateAdditiveRootChannels)
{
    AnimationClip clip;

    MakeClip(clip);

    TranslationChannel translationChannel(0);
    RotationChannel rotationChannel(0);

    clip.ComputeAdditiveRootMotion(translationChannel, rotationChannel);

    ASSERT_EQ(translationChannel.GetKeyCount(), clip.translationChannels[0].GetKeyCount() - 1);
    ASSERT_EQ(rotationChannel.GetKeyCount(), clip.rotationChannels[0].GetKeyCount() - 1);
}

TEST(RootMotionTests, TranslationChannels)
{
    AnimationClip clip;

    MakeClip(clip);

    TranslationChannel translationChannel(0);
    RotationChannel rotationChannel(0);

    clip.ComputeAdditiveRootMotion(translationChannel, rotationChannel);

    ASSERT_EQ(translationChannel.GetKeyCount(), clip.translationChannels[0].GetKeyCount() - 1);

    for (int i = 0; i < translationChannel.GetKeyCount(); ++i)
    {
        auto& key = translationChannel.GetKey(i);

        EXPECT_VEC3_EQ(key.value, vec3(0.0f, 1.0f, 0.0f));
    }
}

TEST(RootMotionTests, RotationChannels)
{
    AnimationClip clip;

    MakeClip(clip);

    TranslationChannel translationChannel(0);
    RotationChannel rotationChannel(0);

    clip.ComputeAdditiveRootMotion(translationChannel, rotationChannel);

    ASSERT_EQ(rotationChannel.GetKeyCount(), clip.rotationChannels[0].GetKeyCount() - 1);

    for (int i = 0; i < rotationChannel.GetKeyCount(); ++i)
    {
        auto& key = rotationChannel.GetKey(i);

        EXPECT_QUAT_EQ(key.value, quat(vec3(0.1f, 0.0f, 0.0f)));
    }
}

// TODO -- test the time normalization...
