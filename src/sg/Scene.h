#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

#include "Light.h"
#include "Model.h"
#include "Camera.h"
#include "ShaderProgram.h"

class Scene
{
    public:
        Scene();

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
};