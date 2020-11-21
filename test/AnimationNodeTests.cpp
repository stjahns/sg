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
#include "MockNode.h"

using namespace glm;

AnimationClip CreateClip();

TEST(ClipNode, AppliesPose_0s)
{
    AnimationClip clip = CreateClip();
    ClipNode node(clip);

    AnimationPose pose;
    node.Evaluate(pose);

    vec3 translation;
    quat rotation;
    EXPECT_TRUE(pose.GetTranslation(1, translation));
    EXPECT_TRUE(pose.GetRotation(1, rotation));

    EXPECT_QUAT_EQ(rotation, quat(1.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_VEC3_EQ(translation, vec3());
}

TEST(ClipNode, AppliesPose_1s)
{
    AnimationClip clip = CreateClip();
    ClipNode node(clip);

    node.Update(1.0f, Parameters());

    AnimationPose pose;
    node.Evaluate(pose);

    vec3 translation;
    quat rotation;
    EXPECT_TRUE(pose.GetTranslation(1, translation));
    EXPECT_TRUE(pose.GetRotation(1, rotation));

    EXPECT_QUAT_EQ(rotation, quat(vec3(half_pi<float>(), 0.0f, 0.0f)));
    EXPECT_VEC3_EQ(translation, vec3(0.0f, 1.0f, 0.0f));
}

TEST(ClipNode, AddPhaseTrigger_SetsParameterOnPhase)
{
    AnimationClip clip = CreateClip();
    clip.duration = 1.0f;

    ClipNode node(clip);

    float phase = 0.95f;
    int id = 12;
    int value = 34;

    node.AddPhaseTrigger({ phase, id, value });

    Parameters p;

    node.Update(0.94f, p);

    EXPECT_EQ(p.size(), 0);

    p.clear();
    node.Update(0.01f, p);

    ASSERT_EQ(p.size(), 1);
    EXPECT_EQ(p.back().id, id);
    EXPECT_EQ(p.back().value, value);
}

TEST(ClipNode, AddPhaseTrigger_DoesntSetParameterAfterLoop)
{
    AnimationClip clip = CreateClip();
    clip.duration = 1.0f;

    ClipNode node(clip);

    float phase = 0.95f;
    int id = 12;
    int value = 34;

    node.AddPhaseTrigger({ phase, id, value });

    Parameters p;

    node.Update(0.95f, p);

    EXPECT_EQ(p.size(), 1);

    p.clear();
    node.Update(0.05f, p);

    EXPECT_EQ(p.size(), 0);
}

TEST(BlendNode, Update_UpdatesChildNodes)
{
    MockNode node1;
    MockNode node2;
    BlendNode blendNode(node1, node2);

    blendNode.Update(69.0f, Parameters());

    EXPECT_EQ(node1.deltaTime, 69.0f);
    EXPECT_EQ(node2.deltaTime, 69.0f);
}

TEST(BlendNode, Blend_0)
{
    AnimationClip clip = CreateClip();

    ClipNode node1(clip);
    ClipNode node2(clip);
    BlendNode blendNode(node1, node2);

    node2.Update(1.0f, Parameters());
    blendNode.SetBlend(0.0f);

    AnimationPose pose;
    blendNode.Evaluate(pose);

    vec3 translation;
    quat rotation;
    EXPECT_TRUE(pose.GetTranslation(1, translation));
    EXPECT_TRUE(pose.GetRotation(1, rotation));

    EXPECT_VEC3_EQ(translation, vec3(0.0f, 0.0f, 0.0f));
    EXPECT_QUAT_EQ(rotation, quat(vec3(0.0, 0.0f, 0.0f)));
}

TEST(BlendNode, Blend_0_5)
{
    AnimationClip clip = CreateClip();

    ClipNode node1(clip);
    ClipNode node2(clip);
    BlendNode blendNode(node1, node2);

    node2.Update(1.0f, Parameters());
    blendNode.SetBlend(0.5f);

    AnimationPose pose;
    blendNode.Evaluate(pose);

    vec3 translation;
    quat rotation;
    EXPECT_TRUE(pose.GetTranslation(1, translation));
    EXPECT_TRUE(pose.GetRotation(1, rotation));

    EXPECT_VEC3_EQ(translation, vec3(0.0f, 0.5f, 0.0f));
    EXPECT_QUAT_EQ(rotation, quat(vec3(quarter_pi<float>(), 0, 0)));
}

AnimationClip CreateClip()
{
    AnimationClip clip;

    BoneIndex boneIndex(1);

    RotationChannel rotationChannel(boneIndex);
    rotationChannel.AddKey(0.0f, quat(vec3(0, 0, 0)));
    rotationChannel.AddKey(1.0f, quat(vec3(half_pi<float>(), 0, 0)));
    clip.rotationChannels.push_back(rotationChannel);

    TranslationChannel translationChannel(boneIndex);
    translationChannel.AddKey(0.0f, vec3(0));
    translationChannel.AddKey(1.0f, vec3(0, 1, 0));
    clip.translationChannels.push_back(translationChannel);

    clip.duration = FLT_MAX;

    return clip;
}
