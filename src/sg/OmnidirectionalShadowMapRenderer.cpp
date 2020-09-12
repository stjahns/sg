#include <sstream>
#include <random>

#include "OmnidirectionalShadowMapRenderer.h"

void OmnidirectionalShadowMapRenderer::RenderLightMaps(Scene& scene, std::function<void(ShaderProgram&)> drawRoutine)
{
    for (int i = 0; i < scene.GetNumPointLights(); ++i)
    {
        RenderLightMap(scene, scene.GetPointLights()[i], drawRoutine);
    }
}

void OmnidirectionalShadowMapRenderer::RenderLightMap(Scene& scene, PointLight& light, std::function<void(ShaderProgram&)> drawRoutine)
{
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, pointLightMapFBO);

    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    float nearPlane = 0.0f;
    float farPlane = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);

    std::vector<glm::mat4> shadowTransforms;

    // view matrices looking in each direction
    // TODO -- can probably precompute these and just update the translations...
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

    for (int j = 0; j < 6; ++j)
    {
        GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + j;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, light.depthCubemap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glClear(GL_DEPTH_BUFFER_BIT);

        shadowMapShader.Use();

        shadowMapShader.SetUniform("shadowMatrix", shadowTransforms[j]);
        shadowMapShader.SetUniform("far_plane", farPlane);
        shadowMapShader.SetUniform("lightPos", light.position);

        drawRoutine(shadowMapShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
