#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

#include "Pose.h"
#include "AnimationNode.h"

using namespace glm;

struct Condition
{
    int parameterId;
    int parameterValue;
};

struct Transition
{
    int sourceState;
    int destinationState;
    float duration;
    Condition condition;
};

class StateMachineNode : public AnimationNode
{

public:

    StateMachineNode()
        : currentState(-1)
        , transitionTimeElapsed(0.0f)
    {
    }

    bool InTransition() const { return transitionNode != nullptr; }
    int GetCurrentState() const { return currentState; }

    virtual void Update(float deltaTime, const Parameters& parameters) override;
    virtual void Evaluate(AnimationPose& pose) override;

    void AddState(std::shared_ptr<AnimationNode> state)
    {
        states.push_back(std::move(state));

        if (states.size() == 1)
        {
            currentState = 0;
        }
    }

    void AddTransition(Transition transition)
    {
        transitions.push_back(transition);
    }

private:

    void UpdateTransition(float deltaTime, const Parameters& parameters);
    void CheckForNewTransition(const Parameters& parameters);
    void StartTransition(const Transition& transition);

    int currentState;

    std::unique_ptr<BlendNode> transitionNode; // this doesn't need to be a pointer if we can swap its children
    Transition currentTransition;
    float transitionTimeElapsed;

    std::vector<std::shared_ptr<AnimationNode>> states;
    std::vector<Transition> transitions;

};
