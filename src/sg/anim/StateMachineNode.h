#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

#include "Pose.h"
#include "AnimationNode.h"

using namespace glm;

struct State
{
    AnimationNode& node;
};

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


    void AddState(State state)
    {
        states.push_back(state);

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

    std::unique_ptr<BlendNode> transitionNode;
    Transition currentTransition;
    float transitionTimeElapsed;

    std::vector<State> states;
    std::vector<Transition> transitions;

};
