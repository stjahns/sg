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

TEST(MatrixTests, Test1)
{
    mat4 m1 = translate(mat4(), vec3(1, 0, 0));
    mat4 m2 = translate(mat4(), vec3(2, 0, 0));
    mat4 result = m1 * m2;
    mat4 expected = translate(mat4(), vec3(3, 0, 0));
    EXPECT_MAT4_EQ(result, expected);
}

TEST(MatrixTests, Test2)
{
    mat4 m1 = translate(mat4(), vec3(1, 0, 0));
    mat4 m2 = rotate(mat4(), pi<float>(), vec3(0, 1, 0));

    vec3 result = (m1 * m2) * vec4(0, 0, 0, 1);
    vec3 expected = vec3(1, 0, 0);

    EXPECT_VEC3_EQ(result, expected);
}

TEST(MatrixTests, Test3)
{
    mat4 m1 = translate(mat4(), vec3(1, 0, 0));
    mat4 m2 = rotate(mat4(), pi<float>(), vec3(0, 1, 0));

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
    pose.localTransforms[0] = translate(mat4(), vec3(1));
    pose.localTransforms[1] = rotate(mat4(), pi<float>(), vec3(1, 0, 0));
    pose.localTransforms[2] = rotate(mat4(), pi<float>(), vec3(1, 0, 0));

    pose.ComputeObjectFromLocal(skeleton);

    ASSERT_EQ(pose.objectTransforms.size(), 3);
    EXPECT_MAT4_NEAR(pose.objectTransforms[2], translate(mat4(), vec3(1)), 0.00001f);
}

TEST(PoseTests, ComputeObjectFromLocal_SingleChain2)
{
    Skeleton skeleton;

    skeleton.bones.emplace_back("root");
    skeleton.bones.emplace_back("bone1", 0);
    skeleton.bones.emplace_back("bone2", 1);

    Pose pose;
    pose.localTransforms.resize(3);
    pose.localTransforms[0] = translate(mat4(), vec3(1, 0, 0));
    pose.localTransforms[1] = scale(mat4(), vec3(2));
    pose.localTransforms[2] = translate(mat4(), vec3(0, 1, 0));

    pose.ComputeObjectFromLocal(skeleton);

    ASSERT_EQ(pose.objectTransforms.size(), 3);

    vec3 endPosition = pose.objectTransforms[2] * vec4(0, 0, 0, 1);

    EXPECT_VEC3_NEAR(endPosition, vec3(1, 2, 0), 0.00001f);
}