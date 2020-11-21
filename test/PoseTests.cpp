#include <stdio.h>
#include <stdio.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"

#include "Pose.h"
#include "Skeleton.h"
#include "AssimpSkeletonLoader.h"
#include "TestHelpers.h"

using namespace glm;

TEST(MatrixTests, Test1)
{
    mat4 m1 = translate(mat4(1.0f), vec3(1, 0, 0));
    mat4 m2 = translate(mat4(1.0f), vec3(2, 0, 0));
    mat4 result = m1 * m2;
    mat4 expected = translate(mat4(1.0f), vec3(3, 0, 0));
    EXPECT_MAT4_EQ(result, expected);
}

TEST(MatrixTests, Test2)
{
    mat4 m1 = translate(mat4(1), vec3(1, 0, 0));
    mat4 m2 = rotate(mat4(1), pi<float>(), vec3(0, 1, 0));

    vec3 result = (m1 * m2) * vec4(0, 0, 0, 1);
    vec3 expected = vec3(1, 0, 0);

    EXPECT_VEC3_EQ(result, expected);
}

TEST(MatrixTests, Test3)
{
    mat4 m1 = translate(mat4(1.0f), vec3(1, 0, 0));
    mat4 m2 = rotate(mat4(1.0f), pi<float>(), vec3(0, 1, 0));

    vec3 result = (m2 * m1) * vec4(0, 0, 0, 1);
    vec3 expected = vec3(-1, 0, 0);

    EXPECT_VEC3_NEAR(result, expected, 0.00001);
}

TEST(PoseTests, ComputeObjectFromLocal_SingleChain1)
{
    Skeleton skeleton;

    skeleton.bones.emplace_back("root");
    skeleton.bones.emplace_back("bone1", 0);
    skeleton.bones.emplace_back("bone2", 1);

    Pose pose;
    pose.localTransforms.resize(3);
    pose.localTransforms[0] = translate(mat4(1.0f), vec3(1));
    pose.localTransforms[1] = rotate(mat4(1.0f), pi<float>(), vec3(1, 0, 0));
    pose.localTransforms[2] = rotate(mat4(1.0f), pi<float>(), vec3(1, 0, 0));

    pose.ComputeObjectFromLocal(skeleton);

    ASSERT_EQ(pose.objectTransforms.size(), 3);
    EXPECT_MAT4_NEAR(pose.objectTransforms[2], translate(mat4(1.0f), vec3(1)), 0.00001f);
}

TEST(PoseTests, ComputeObjectFromLocal_SingleChain2)
{
    Skeleton skeleton;

    skeleton.bones.emplace_back("root");
    skeleton.bones.emplace_back("bone1", 0);
    skeleton.bones.emplace_back("bone2", 1);

    Pose pose;
    pose.localTransforms.resize(3);
    pose.localTransforms[0] = translate(mat4(1.0f), vec3(1, 0, 0));
    pose.localTransforms[1] = scale(mat4(1.0f), vec3(2));
    pose.localTransforms[2] = translate(mat4(1.0f), vec3(0, 1, 0));

    pose.ComputeObjectFromLocal(skeleton);

    ASSERT_EQ(pose.objectTransforms.size(), 3);

    vec3 endPosition = pose.objectTransforms[2] * vec4(0, 0, 0, 1);

    EXPECT_VEC3_NEAR(endPosition, vec3(1, 2, 0), 0.00001f);
}

vec3 GetTranslation(mat4 m)
{
    return m[3];
}

TEST(PoseTests, AnimationPoseToSkeletonPose_NoValidTransforms_NoChange)
{
    Skeleton skeleton;
    skeleton.bones.emplace_back("root");
    skeleton.bones.emplace_back("bone1", 0);
    skeleton.bones.emplace_back("bone2", 1);

    AnimationPose animationPose;

    Pose skeletonPose;
    skeletonPose.localTransforms.resize(3);

    skeleton.ApplyAnimationPose(animationPose, skeletonPose);

    ASSERT_EQ(skeletonPose.localTransforms.size(), 3);
    EXPECT_VEC3_EQ(GetTranslation(skeletonPose.localTransforms[0]), vec3(0.0f, 0.0f, 0.0f));
    EXPECT_VEC3_EQ(GetTranslation(skeletonPose.localTransforms[0]), vec3(0.0f, 0.0f, 0.0f));
    EXPECT_VEC3_EQ(GetTranslation(skeletonPose.localTransforms[0]), vec3(0.0f, 0.0f, 0.0f));
}

TEST(PoseTests, AnimationPoseToSkeletonPose)
{
    Skeleton skeleton;
    skeleton.bones.emplace_back("root");
    skeleton.bones.emplace_back("bone1", 0);
    skeleton.bones.emplace_back("bone2", 1);

    AnimationPose animationPose;
    animationPose.SetTranslation(0, vec3(1.0f));
    animationPose.SetTranslation(1, vec3(2.0f));
    animationPose.SetTranslation(2, vec3(3.0f));

    Pose skeletonPose;
    skeletonPose.localTransforms.resize(3);

    skeleton.ApplyAnimationPose(animationPose, skeletonPose);

    ASSERT_EQ(skeletonPose.localTransforms.size(), 3);
    EXPECT_VEC3_EQ(GetTranslation(skeletonPose.localTransforms[0]), vec3(1.0f, 1.0f, 1.0f));
    EXPECT_VEC3_EQ(GetTranslation(skeletonPose.localTransforms[1]), vec3(2.0f, 2.0f, 2.0f));
    EXPECT_VEC3_EQ(GetTranslation(skeletonPose.localTransforms[2]), vec3(3.0f, 3.0f, 3.0f));
}

// TODO -- can we have a valid rotation animation that doesn't affect the default skeleton translation?