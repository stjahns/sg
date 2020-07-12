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

        void AddWidgets();

        void DrawLights();

        Camera& GetCamera() { return camera; }

        PointLight* GetPointLights() { return pointLights; }
        int GetNumPointLights() { return numPointLights; };

        SpotLight* GetSpotLights() { return spotLights; }
        int GetNumSpotLights() { return numSpotLights; }

        DirectionalLight& GetDirectionalLight() { return directionalLight; }

        static const int maxPointLights = 4;
        static const int maxSpotLights = 4;

    private:

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