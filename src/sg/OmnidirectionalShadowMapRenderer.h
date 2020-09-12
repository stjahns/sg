#pragma once

#include <vector>

#include "Light.h"
#include "Model.h"
#include "Camera.h"
#include "ShadowMap.h"
#include "Scene.h"
#include "ShaderProgram.h"

class OmnidirectionalShadowMapRenderer
{
public:

    OmnidirectionalShadowMapRenderer() : shadowMapShader("shaders/omnidirectional_map.vs.glsl", "shaders/omnidirectional_map.fs.glsl")
    {
        glGenFramebuffers(1, &pointLightMapFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, pointLightMapFBO);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Renders to each light.depthCubeMap
    // TODO Should try to make inputs/outputs more explicit...

    void RenderLightMaps(Scene& scene, std::function<void(ShaderProgram&)> drawRoutine);
    void RenderLightMap(Scene& scene, PointLight& light, std::function<void(ShaderProgram&)> drawRoutine);

    // TODO might want to allow these to be dynamic
    static const int SHADOW_WIDTH = 1024;
    static const int SHADOW_HEIGHT = 1024;

private:

    ShaderProgram shadowMapShader;
    GLuint pointLightMapFBO;

};
