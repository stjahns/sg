#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

#include "Light.h"
#include "Model.h"
#include "Camera.h"
#include "ShaderProgram.h"
#include "ModelLoader.h"

class GLFWwindow;

class Scene
{
    public:
        Scene() 
            : lightModel("models/glTF/box/box.gltf")
            , numPointLights(0)
            , numSpotLights(0)
            , lightShader("shaders/mvp.vs.glsl", "shaders/white.fs.glsl")
        {
            lightModel.Load();

            /*
            Model* box  = new Model("glTF/box/box.gltf");

            glm::mat4 t = glm::mat4(1);
            t = glm::translate(t, vec3(0, -6.5, 0));
            t = glm::scale(t, vec3(10));
            box->SetTransform(t);

            modelsToLoad.push_back(box);
            modelsToLoad.push_back(new Model("nanosuit/nanosuit.obj"));
            */
            //modelsToLoad.push_back(new Model("gltf/sponza.gltf"));

            //SponzaTest();
            //NanoSuitTest();
            CornellBoxTest();
        }

        void SponzaTest()
        {
            loader.Load("Models/gltf/sponza.gltf");
        }

        void NanoSuitTest()
        {
            loader.Load("Models/nanosuit/nanosuit.obj");
        }

        void CornellBoxTest()
        {
            loader.Load("Models/cornell-box/cornellbox-glossy.obj");
            //modelsToLoad.push_back(new Model("cornell-box/cornellbox-noobjs.obj"));
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

        void LoadModels();
        void Update();

        void AddModel(Model *pModel)
        {
            models.push_back(pModel);
        }

        void AddWidgets();

        void DrawLights();

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

        ModelLoader loader;
};