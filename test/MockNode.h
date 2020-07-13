#pragma once

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

struct MockNode : public AnimationNode
{
    MockNode() : deltaTime(0.0f), reset(0)
    {
    }

    float deltaTime;
    int reset;

    vec3 translation;
    quat rotation;

    virtual void Update(float deltaTime, Parameters& p) override { this->deltaTime = deltaTime; }
    virtual void Reset() override { ++reset; }
    virtual void Evaluate(AnimationPose& poseOut) override
    {
        poseOut.SetTranslation(0, translation);
        poseOut.SetRotation(0, rotation);
    }
};
