#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Pose.h"
#include "AnimationClip.h"

using namespace glm;

struct Parameter
{
    int id;
    int value;
};

typedef std::vector<Parameter> Parameters;

class AnimationNode
{
public:
    virtual void Update(float deltaTime, const Parameters& parameters) = 0;
    virtual void Evaluate(AnimationPose& pose) = 0;
};

class ClipNode : public AnimationNode
{
public:

    ClipNode(const AnimationClip& clip) : clip(clip), time(0.0f)
    {
    }

    virtual void Update(float deltaTime, const Parameters& parameters) override
    {
        time = fmodf(time + deltaTime, clip.duration);
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
        , blend(0.0f)
    {
    }

    virtual void Update(float deltaTime, const Parameters& parameters) override
    {
        node1.Update(deltaTime, parameters);
        node2.Update(deltaTime, parameters);
    }

    virtual void Evaluate(AnimationPose& pose) override;

    void SetBlend(float blend) { this->blend = blend; }

private:

    float blend;
    AnimationNode& node1;
    AnimationNode& node2;
};
