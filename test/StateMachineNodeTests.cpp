#include <stdio.h>
#include <gtest/gtest.h>
// TODO #include <gmock/gmock.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AnimationNode.h"
#include "StateMachineNode.h"
#include "TestHelpers.h"

using namespace glm;

namespace
{
    struct MockNode : public AnimationNode
    {
        MockNode() : deltaTime(0.0f) {}

        float deltaTime;

        vec3 translation;
        quat rotation;

        virtual void Update(float deltaTime, const Parameters& parameters) override { this->deltaTime += deltaTime; }
        virtual void Evaluate(AnimationPose& poseOut) override 
        { 
            poseOut.SetTranslation(0, translation);
            poseOut.SetRotation(0, rotation);
        }
    };
}

TEST(StateMachineNode, InitialStateIsFirstStateAdded)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    Parameters parameters;

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    EXPECT_EQ(stateMachine.GetCurrentState(), 0);
}

TEST(StateMachineNode, UpdatesInitialState)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    Parameters parameters;

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    stateMachine.Update(1.0f, parameters);
    
    EXPECT_EQ(node1.deltaTime, 1.0f);
    EXPECT_EQ(node2.deltaTime, 0.0f);
}

TEST(StateMachineNode, EvaluatesInitialState)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    Parameters parameters;

    node1.translation = vec3(1.0f, 2.0f, 3.0f);

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    AnimationPose pose;
    stateMachine.Evaluate(pose);
    
    vec3 t;
    pose.GetTranslation(0, t);

    EXPECT_EQ(t, node1.translation);
}

TEST(StateMachineNode, Update_WhenConditionNotMet_DoesNotTriggerTransition)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    Condition condition{ 420, 1337 };
    Transition transition{ 0, 1, 1.0f, condition };
    stateMachine.AddTransition(transition);

    Parameters parameters{ { 0, 1 } };
    stateMachine.Update(0.0f, parameters);

    EXPECT_EQ(stateMachine.InTransition(), false);
}

TEST(StateMachineNode, Update_WhenConditionMet_TriggersTransition)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    Condition condition{ 420, 1337 };
    Transition transition{ 0, 1, 1.0f, condition };
    stateMachine.AddTransition(transition);

    Parameters parameters{ { 420, 1337 } };
    stateMachine.Update(0.0f, parameters);

    EXPECT_EQ(stateMachine.InTransition(), true);
}

TEST(StateMachineNode, Update_DuringTransition_UpdatesBothStateNodes)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    Condition condition{ 420, 1337 };
    Transition transition{ 0, 1, 1.0f, condition };
    stateMachine.AddTransition(transition);

    Parameters parameters{ { 420, 1337 } };
    stateMachine.Update(0.0f, parameters);
    stateMachine.Update(1.0f, parameters);

    EXPECT_EQ(node1.deltaTime, 1.0f);
    EXPECT_EQ(node2.deltaTime, 1.0f);
}

TEST(StateMachineNode, Evaluate_DuringTransition_BlendsStateOutputs)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    node1.translation = vec3(0.0f);
    node2.translation = vec3(1.0f);

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    Condition condition{ 420, 1337 };
    Transition transition{ 0, 1, 1.0f, condition };
    stateMachine.AddTransition(transition);

    Parameters parameters{ { 420, 1337 } };
    stateMachine.Update(0.0f, parameters); // start transition
    stateMachine.Update(0.5f, parameters); // transition midpoint

    AnimationPose pose;
    stateMachine.Evaluate(pose);
    
    vec3 t;
    pose.GetTranslation(0, t);

    EXPECT_VEC3_EQ(t, vec3(0.5f));
}

TEST(StateMachineNode, Update_DuringTransition_EndsTransition)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    Condition condition{ 420, 1337 };
    Transition transition{ 0, 1, 1.0f, condition };
    stateMachine.AddTransition(transition);

    Parameters parameters{ { 420, 1337 } };

    stateMachine.Update(0.0f, parameters); // start transition
    stateMachine.Update(1.0f, parameters); // finish transition

    EXPECT_EQ(stateMachine.InTransition(), false);
}

TEST(StateMachineNode, Update_DuringTransition_EntersState)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    Condition condition{ 420, 1337 };
    Transition transition{ 0, 1, 1.0f, condition };
    stateMachine.AddTransition(transition);

    Parameters parameters{ { 420, 1337 } };

    stateMachine.Update(0.0f, parameters); // start transition
    stateMachine.Update(1.0f, parameters); // finish transition

    EXPECT_EQ(stateMachine.GetCurrentState(), 1);
}

TEST(StateMachineNode, Update_AfterTransition_UpdatesDestinationState)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    Condition condition{ 420, 1337 };
    Transition transition{ 0, 1, 1.0f, condition };
    stateMachine.AddTransition(transition);

    Parameters parameters{ { 420, 1337 } };

    stateMachine.Update(0.0f, parameters); // start transition
    stateMachine.Update(1.0f, parameters); // do transition

    node1.deltaTime = 0.0f;
    node2.deltaTime = 0.0f;

    stateMachine.Update(1.0f, parameters); // in target state

    EXPECT_EQ(node1.deltaTime, 0.0f);
    EXPECT_EQ(node2.deltaTime, 1.0f);
}

TEST(StateMachineNode, Evaluate_AfterTransition_EvaluatesDestinationState)
{
    StateMachineNode stateMachine;

    MockNode node1, node2;
    State state1{ node1 }, state2{ node2 };

    stateMachine.AddState(state1);
    stateMachine.AddState(state2);

    Condition condition{ 420, 1337 };
    Transition transition{ 0, 1, 1.0f, condition };
    stateMachine.AddTransition(transition);

    Parameters parameters{ { 420, 1337 } };

    stateMachine.Update(0.0f, parameters); // start transition
    stateMachine.Update(1.0f, parameters); // do transition

    node1.deltaTime = 0.0f;
    node2.deltaTime = 0.0f;

    stateMachine.Update(1.0f, parameters); // in target state

    node2.translation = vec3(1.0f);

    AnimationPose pose;
    stateMachine.Evaluate(pose);
    
    vec3 t;
    pose.GetTranslation(0, t);

    EXPECT_EQ(t, node2.translation);
}
