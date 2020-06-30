#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "ShaderProgram.h"

class ShadowMap
{
    public:

    ShadowMap() 
        : shader("shaders/shadowmap.vs.glsl", "shaders/empty.fs.glsl")
        , NEAR_PLANE(1.0f)
        , FAR_PLANE(11.0f)
        , ORTHO_WIDTH(10.0f)
    {
        glGenFramebuffers(1, &fbo);

        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~ShadowMap()
    {
        // cleanup framebuffers/textures
    }

    void PreRender(glm::vec3 lightDirection)
    {
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        shader.Use();
        shader.SetUniform("lightSpaceMatrix", GetLightSpaceMatrix(lightDirection));
    }

    glm::mat4 GetLightSpaceMatrix(glm::vec3 lightDirection)
    {
        // IMPORTANT! light view needs to be placed above anything that can cast a shadow..
        glm::mat4 lightProjection = glm::ortho(-ORTHO_WIDTH, ORTHO_WIDTH, -ORTHO_WIDTH, ORTHO_WIDTH, NEAR_PLANE, FAR_PLANE);
        glm::mat4 lightView = glm::lookAt(-lightDirection * 10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        return lightSpaceMatrix;
    }

    void PostRender()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void SetModel(glm::mat4 model)
    {
        shader.SetUniform("model", model);
    }

    GLuint GetDepthMap() { return depthMap; }

    GLfloat NEAR_PLANE;
    GLfloat FAR_PLANE;
    GLfloat ORTHO_WIDTH;

    private:

    static const GLuint SHADOW_WIDTH = 1024;
    static const GLuint SHADOW_HEIGHT = 1024;

    GLuint fbo;
    GLuint depthMap;
    ShaderProgram shader;

};