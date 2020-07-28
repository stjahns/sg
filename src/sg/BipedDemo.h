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
#include "Window.h"

#include "anim/Skeleton.h"
#include "anim/AnimationClip.h"
#include "anim/AnimationNode.h"
#include "anim/StateMachineNode.h"

class BipedDemo
{
public:

    BipedDemo(Camera& camera);

    void Update(float deltaTime);
    void Render();

    void AddWidgets();
    void Reload() { shaderProgram.Reload(); }

private:

    void LoadSkeletonFromFile(std::string filename, Skeleton& skeleton);
    void LoadClipFromFile(std::string filename, Skeleton& skeleton, int clipIndex = 0, std::string idPrefix = "");

    void LoadBiped();

    Scene scene;

    entt::registry entityRegistry;

    std::vector<Model> models;

    Camera& camera;
    Renderer renderer;
    LineRenderer lineRenderer;

    ShaderProgram shaderProgram;

    Skeleton skeleton;

    std::vector<AnimationClip> clips;

    std::unique_ptr<ClipNode> clipNode1;
    std::unique_ptr<ClipNode> clipNode2;
    std::unique_ptr<StateMachineNode> stateMachine;

    double time;
    float timescale;

    bool drawWireframe;
    bool drawSkeleton;

    std::vector<std::string> skinUniformIds;
};