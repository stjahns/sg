#include "SceneDemo.h"
#include "Light.h"

SceneDemo::SceneDemo(Window& window, Camera& camera)
    : camera(camera)
    , renderer(window)
{
    LoadScene();
}

void SceneDemo::LoadScene()
{
    Model& model = models.emplace_back("Models/cornell-box/cornellbox-glossy.obj");
    model.Load();

    scene.GetDirectionalLight().diffuse = vec3(0);
    scene.GetDirectionalLight().specular = vec3(0);
    scene.GetDirectionalLight().ambient = vec3(0);

    // this sucks
    scene.AddPointLight();
    scene.GetPointLights()[0].position.y = 2.0f;
    scene.GetPointLights()[0].constant = 0.0f;
    scene.GetPointLights()[0].linear = 0.0f;
    scene.GetPointLights()[0].quadratic = 1.0f;

    camera.SetPosition(vec3(0, 1.0, 3.5));
    camera.SetSpeed(0.1f);
}

void SceneDemo::Update(float deltaTime)
{
}

void SceneDemo::Render()
{
    shadowMapRenderer.RenderLightMaps(scene, [&](ShaderProgram& shader)
    {
        for (auto& model : models)
        {
            shader.SetUniform("model", model.GetTransform());
            model.Draw();
        }
    });

    // TODO -- handle this properly
    for (auto& model : models)
    {
        renderer.DeferredRender(scene, camera, model);
    }
}

void SceneDemo::AddWidgets()
{
    scene.AddWidgets();
    renderer.AddWidgets();
}

void SceneDemo::Reload()
{
    renderer.ReloadShaders();
}
