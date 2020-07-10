#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Pose.h"
#include "AnimationClip.h"

using namespace glm;

class AnimationNode
{
public:
    virtual void Update(float deltaTime) = 0;
    virtual void Evaluate(AnimationPose& pose) = 0;
};

class ClipNode : public AnimationNode
{
public:

    ClipNode(const AnimationClip& clip) : clip(clip), time(0.0f)
    {
    }

    virtual void Update(float deltaTime) override
    {
        time += deltaTime;
    }

    virtual void Evaluate(AnimationPose& pose) override;

private:

    float time;

    // TODO -- how to enforce ownership/lifetimes ... use sharedptr?
    const AnimationClip& clip;
};

class BlendNode : public AnimationNode
{
public:

    BlendNode(AnimationNode& node1, AnimationNode& node2)
        : node1(node1)
        , node2(node2)
        , blend(1.0f)
    {
    }

    virtual void Update(float deltaTime) override
    {
        node1.Update(deltaTime);
        node2.Update(deltaTime);
    }

    virtual void Evaluate(AnimationPose& pose) override;

    void SetNode1Weight(float weight) { blend = weight; }
    void SetNode2Weight(float weight) { blend = 1.0f - weight; }

private:

    float blend;
    AnimationNode& node1;
    AnimationNode& node2;

};

class StateMachineNode : public AnimationNode { };
