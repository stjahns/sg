#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

#include "Light.h"
#include "Model.h"
#include "Camera.h"
#include "ShaderProgram.h"

#include "anim/Skeleton.h"
#include "anim/AnimationClip.h"

class GLFWwindow;
class LineRenderer;

class Scene
{
    public:
        Scene();

        void SponzaTest()
        {
            LoadModel("Models/gltf/sponza.gltf");
        }

        void NanoSuitTest()
        {
            LoadModel("Models/nanosuit/nanosuit.obj");
        }

        void CornellBoxTest()
        {
            LoadModel("Models/cornell-box/cornellbox-glossy.obj");

            directionalLight.diffuse = vec3(0);
            directionalLight.specular = vec3(0);
            directionalLight.ambient = vec3(0);
            numPointLights = 1;

            pointLights[0].position.y = 2.0f;
            pointLights[0].constant = 0.0f;
            pointLights[0].linear = 0.0f;
            pointLights[0].quadratic = 1.0f;

            camera.SetPosition(vec3(0, 1.0, 3.5));

            // DONE - Gamma correction
            // TODO - Fix point shadow frustums
            // TODO - Blur postprocessing
            // TODO - Blinn-phong
            // TODO - SSAO
            // TODO - Soft shadow mapping
            // TODO - Anti aliasing - Forward Rendering + MSAA?
            // TODO - Anti aliasing - FXAA?
            // TODO - HDR
            // TODO - Bloom?
        }

        void Update(LineRenderer& lineRenderer, ShaderProgram& shader);

        void LoadModel(const char* path)
        {
            Model* model = new Model(path);
            model->Load();
            AddModel(model);
        }

        void AddModel(Model *pModel)
        {
            models.push_back(pModel);
        }

        void AddWidgets();

        void DrawLights();
        void DebugDraw();

        Camera& GetCamera() { return camera; }

        std::vector<Model *>& GetModels() { return models; }

        PointLight* GetPointLights() { return pointLights; }
        int GetNumPointLights() { return numPointLights; };

        SpotLight* GetSpotLights() { return spotLights; }
        int GetNumSpotLights() { return numSpotLights; }

        DirectionalLight& GetDirectionalLight() { return directionalLight; }

        static const int maxPointLights = 4;
        static const int maxSpotLights = 4;

    private:
        std::vector<Model *> models;
        std::vector<Model *> modelsToLoad;

        int numPointLights;
        int numSpotLights;

        DirectionalLight directionalLight;
        PointLight pointLights[maxPointLights];
        SpotLight spotLights[maxSpotLights];

        const float lightScale = 0.2f;
        Model lightModel;
        ShaderProgram lightShader;

        Camera camera;

        Skeleton skeleton;
        AnimationClip clip;
        float tick;
};