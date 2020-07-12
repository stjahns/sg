#include "StateMachineNode.h"

namespace
{
    bool CheckCondition(const Parameters& parameters, Condition condition)
    {
        for (auto const& parameter : parameters)
        {
            if (parameter.id == condition.parameterId &&
                parameter.value == condition.parameterValue)
            {
                return true;
            }
        }

        return false;
    }
}

void StateMachineNode::Update(float deltaTime, const Parameters& parameters)
{
    if (currentState != -1)
    {
        states[currentState]->Update(deltaTime, parameters);
    }

    if (transitionNode)
    {
        UpdateTransition(deltaTime, parameters);
    }
    else
    {
        CheckForNewTransition(parameters);
    }
}

void StateMachineNode::Evaluate(AnimationPose& pose)
{
    if (transitionNode)
    {
        transitionNode->Evaluate(pose);
    }
    else if (currentState != -1)
    {
        states[currentState]->Evaluate(pose);
    }
}

void StateMachineNode::UpdateTransition(float deltaTime, const Parameters& parameters)
{
    transitionNode->Update(deltaTime, parameters);
    transitionTimeElapsed += deltaTime;

    float transitionProgress = transitionTimeElapsed / currentTransition.duration;
    if (transitionProgress < 1.0f)
    {
        transitionNode->SetBlend(transitionProgress);
    }
    else
    {
        transitionNode.reset();
        currentState = currentTransition.destinationState;
    }
}

void StateMachineNode::CheckForNewTransition(const Parameters& parameters)
{
    for (const auto& transition : transitions)
    {
        if (transition.sourceState == currentState &&
            CheckCondition(parameters, transition.condition))
        {
            StartTransition(transition);
            return;
        }
    }
}

void StateMachineNode::StartTransition(const Transition& transition)
{
    // TODO -- is this ok? are the node lifetimes appropriate?
    AnimationNode& node1 = *states[transition.sourceState];
    AnimationNode& node2 = *states[transition.destinationState];
    transitionNode = std::make_unique<BlendNode>(node1, node2);
    transitionNode->SetBlend(0.0f);
    transitionTimeElapsed = 0.0f;
    currentTransition = transition;
    currentState = -1;
}
