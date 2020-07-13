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

// is this more a "blackboard" now that nodes are writing to it?
typedef std::vector<Parameter> Parameters;

class AnimationNode
{
public:
    virtual void Reset() = 0;
    virtual void Update(float deltaTime, Parameters& parameters) = 0;
    virtual void Evaluate(AnimationPose& pose) = 0;
};

struct PhaseTrigger
{
    float phase;
    int id;
    int value;
};

class ClipNode : public AnimationNode
{
public:

    ClipNode(const AnimationClip& clip) : clip(clip), time(0.0f)
    {
    }

    virtual void Reset() override
    {
        SetPhase(0.0f);
    }

    virtual void Update(float deltaTime, Parameters& parameters) override
    {
        time = fmodf(time + deltaTime, clip.duration);

        float phase = GetPhase();

        for (const auto& trigger : phaseTriggers)
        {
            if (phase >= trigger.phase)
            {
                parameters.push_back({ trigger.id, trigger.value });
            }
        }
    }

    virtual void Evaluate(AnimationPose& pose) override;

    void SetPhase(float phase)
    {
        SetTime(phase * clip.duration);
    }

    void SetTime(float t)
    {
        time = t;
    }

    float GetPhase() { return time / clip.duration; }

    void AddPhaseTrigger(PhaseTrigger phaseTrigger)
    {
        phaseTriggers.push_back(phaseTrigger);
    }

private:

    float time;

    // TODO -- how to enforce ownership/lifetimes ... use sharedptr?
    const AnimationClip& clip;

    std::vector<PhaseTrigger> phaseTriggers;
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

    virtual void Reset() override 
    {
        node1.Reset();
        node2.Reset();
    }

    virtual void Update(float deltaTime, Parameters& parameters) override
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
