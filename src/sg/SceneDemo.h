#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <entt.hpp>

#include "Model.h"
#include "Scene.h"
#include "DeferredRenderer.h"
#include "OmnidirectionalShadowMapRenderer.h"
#include "Window.h"

class SceneDemo
{
public:

    SceneDemo(Window& window, Camera& camera);

    void Update(float deltaTime);
    void Render();

    void AddWidgets();
    void Reload();

private:

    void LoadScene();

    Camera& camera;
    DeferredRenderer renderer;
    Scene scene;

    OmnidirectionalShadowMapRenderer shadowMapRenderer;

    std::vector<Model> models;
};