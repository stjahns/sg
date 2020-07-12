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

	channel.AddKey(0.0f, expected);

	vec3 v = channel.Evaulate(1.0f);

	EXPECT_VEC3_EQ(v, expected);
}

TEST(PositionChannelTests, TwoKeys)
{
	TranslationChannel channel;

	channel.AddKey(0.0f, vec3(0));
	channel.AddKey(1.0f, vec3(1));

	EXPECT_VEC3_EQ(channel.Evaulate(0.0f), vec3(0));
	EXPECT_VEC3_EQ(channel.Evaulate(1.0f), vec3(1));
}

TEST(PositionChannelTests, TwoKeys_Interpolates)
{
	TranslationChannel channel;

	channel.AddKey(0.0f, vec3(0));
	channel.AddKey(1.0f, vec3(1));

	EXPECT_VEC3_EQ(channel.Evaulate(0.5f), vec3(0.5));
}

TEST(PositionChannelTests, ThreeKeys_ConstantSampleRate_Interpolates)
{
	TranslationChannel channel;

	channel.AddKey(0.0f, vec3(1.0f));
	channel.AddKey(10.0f, vec3(0.0f));
	channel.AddKey(20.0f, vec3(10.0f));

	EXPECT_VEC3_EQ(channel.Evaulate(5.0f), vec3(0.5f));
	EXPECT_VEC3_EQ(channel.Evaulate(15.0f), vec3(5.0f));
}

TEST(PositionChannelTests, TenKeys_VariableSampleRate_Interpolates)
{
	TranslationChannel channel;

	channel.AddKey(0.0f, vec3(1.0f));
	channel.AddKey(1.0f, vec3(1.0f));
	channel.AddKey(2.0f, vec3(1.0f));
	channel.AddKey(3.0f, vec3(1.0f));
	channel.AddKey(4.0f, vec3(1.0f));
	channel.AddKey(5.0f, vec3(0.0f));
	channel.AddKey(10.0f, vec3(0.0f));
	channel.AddKey(10.0f, vec3(0.0f));
	channel.AddKey(10.0f, vec3(0.0f));
	channel.AddKey(30.0f, vec3(10.0f));

	channel.Resample();

	EXPECT_VEC3_EQ(channel.Evaulate(4.5f), vec3(0.5f));
	EXPECT_VEC3_EQ(channel.Evaulate(20.0f), vec3(5.0f));
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

	channel.AddKey(0.0f, expected);

	quat q = channel.Evaulate(1.0f);

	EXPECT_QUAT_EQ(q, expected);
}

TEST(RotationChannelTests, TwoKeys)
{
	RotationChannel channel;

	quat qStart = quat(vec3(0, 0, 0));
	quat qEnd = quat(vec3(1, 0, 0));

	channel.AddKey(0.0f, qStart);
	channel.AddKey(1.0f, qEnd);

	EXPECT_QUAT_EQ(channel.Evaulate(0.0f), qStart);
	EXPECT_QUAT_EQ(channel.Evaulate(1.0f), qEnd);
}

TEST(RotationChannelTests, TwoKeys_Interpolates)
{
	RotationChannel channel;

	quat qStart = quat(vec3(0, 0, 0));
	quat qEnd = quat(vec3(1, 0, 0));

	channel.AddKey(0.0f, qStart);
	channel.AddKey(1.0f, qEnd);

	EXPECT_QUAT_EQ(channel.Evaulate(0.5f), quat(vec3(0.5, 0, 0)));
}

TEST(RotationChannelTests, ThreeKeys_ConstantSampleRate_Interpolates)
{
	RotationChannel channel;


	channel.AddKey(0.0f, quat(vec3(0, 1, 0)));
    channel.AddKey(10.0f, quat(vec3(0, 0, 0)));
	channel.AddKey(20.0f, quat(vec3(2, 0, 0)));

	EXPECT_QUAT_EQ(channel.Evaulate(5.0f), quat(vec3(0.0, 0.5, 0)));
	EXPECT_QUAT_EQ(channel.Evaulate(15.0f), quat(vec3(1.0, 0, 0)));
	EXPECT_QUAT_EQ(channel.Evaulate(20.0f), quat(vec3(2.0, 0, 0)));
}

TEST(RotationChannelTests, TenKeys_VariableSampleRate_Interpolates)
{
	RotationChannel channel;

	channel.AddKey(0.0f, quat(vec3(0, 1, 0)));
    channel.AddKey(1.0f, quat(vec3(0, 1, 0)));
    channel.AddKey(2.0f, quat(vec3(0, 1, 0)));
    channel.AddKey(3.0f, quat(vec3(0, 1, 0)));
    channel.AddKey(4.0f, quat(vec3(0, 1, 0)));
    channel.AddKey(5.0f, quat(vec3(0, 0, 0)));
    channel.AddKey(10.0f, quat(vec3(0, 0, 0)));
    channel.AddKey(10.0f, quat(vec3(0, 0, 0)));
	channel.AddKey(10.0f, quat(vec3(0, 0, 0)));
	channel.AddKey(30.0f, quat(vec3(2, 0, 0)));

	channel.Resample();

	EXPECT_QUAT_EQ(channel.Evaulate(4.5f), quat(vec3(0.0, 0.5, 0)));
	EXPECT_QUAT_EQ(channel.Evaulate(20.0f), quat(vec3(1.0, 0, 0)));
	EXPECT_QUAT_EQ(channel.Evaulate(30.0f), quat(vec3(2.0, 0, 0)));
}
