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

    EXPECT_QUAT_EQ(rotation, quat());
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

namespace
{
    struct MockNode : public AnimationNode
    {
        float deltaTime;

        virtual void Update(float deltaTime, const Parameters& p) override { this->deltaTime = deltaTime; }
        virtual void Evaluate(AnimationPose& pose) override { }
    };
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
    rotationChannel.keys.emplace_back(0.0f, quat(vec3(0, 0, 0)));
    rotationChannel.keys.emplace_back(1.0f, quat(vec3(half_pi<float>(), 0, 0)));
    clip.rotationChannels.push_back(rotationChannel);

    TranslationChannel translationChannel(boneIndex);
    translationChannel.keys.emplace_back(0.0f, vec3(0));
    translationChannel.keys.emplace_back(1.0f, vec3(0, 1, 0));
    clip.translationChannels.push_back(translationChannel);

    return clip;
}
