#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <entt.hpp>

#include "Light.h"
#include "Model.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Renderer.h"
#include "LineRenderer.h"
#include "GlfwWindow.h"

#include "anim/Skeleton.h"
#include "anim/AnimationClip.h"
#include "anim/AnimationNode.h"
#include "anim/StateMachineNode.h"

class Demo
{
    public:

        Demo(GLFWWindow& window);

        void Run();

    private:

        void Update(float deltaTime);
        void Widgets();

        void LoadFox();

        void ForwardRenderModels();

        GLFWWindow& window;

        Scene scene;

        entt::registry entityRegistry;

        std::vector<Model> models;

        Renderer renderer;
        LineRenderer lineRenderer;

        ShaderProgram shaderProgram;

        Skeleton skeleton;
        AnimationClip clip1;
        AnimationClip clip2;

        std::unique_ptr<ClipNode> clipNode1;
        std::unique_ptr<ClipNode> clipNode2;
        std::unique_ptr<StateMachineNode> stateMachine;

        double time;

        bool drawWireframe;

        int p1;
};