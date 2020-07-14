#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <entt.hpp>

#include "Model.h"
#include "Scene.h"
#include "Renderer.h"

class SceneDemo
{
public:

    SceneDemo(Camera& camera);

    void Update(float deltaTime);
    void Render();

    void AddWidgets();
    void Reload() { }

private:

    void LoadScene();

    Camera& camera;
    Renderer renderer;
    Scene scene;

    std::vector<Model> models;
};