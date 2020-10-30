#pragma once

#include <vector>

#include "Light.h"
#include "Model.h"
#include "Camera.h"
#include "ShadowMap.h"
#include "Scene.h"
#include "ShaderProgram.h"

GLenum glCheckError_(const char* file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__) 
//#define glCheckError() 

class DeferredRenderer
{
public:

    DeferredRenderer(Window& window)
        : drawShadowDepth(false)
        , drawGBuffer(false)
        , drawHDRBuffer(false)
        , renderDirectionalShadowMap(false)
        , renderPointShadowMaps(true)
        , pointLightShadowShader("shaders/omnidirectional_map.vs.glsl", "shaders/omnidirectional_map.fs.glsl")
        , fullscreenShader("shaders/quad.vs.glsl", "shaders/fullscreen.fs.glsl")
        //  , geometryPassShader("shaders/deferred.vs.glsl", "shaders/deferred_textured.fs.glsl")
        , geometryPassShader("shaders/deferred.vs.glsl", "shaders/deferred.fs.glsl")
        , ssaoShader("shaders/fullscreen.vs.glsl", "shaders/ssao.fs.glsl")
        , deferredShadingPassShader("shaders/fullscreen.vs.glsl", "shaders/deferredshading.fs.glsl")
        , hdrShader("shaders/fullscreen.vs.glsl", "shaders/hdr.fs.glsl")
        , postShader("shaders/fullscreen.vs.glsl", "shaders/postprocessing.fs.glsl")
        , ssaoEnabled(true)
        , ssaoRadius(0.5)
        , ssaoKernelSize(16)
        , ssaoBias(0.025)
        , ssaoBlurShader("shaders/fullscreen.vs.glsl", "shaders/ssao_blur.fs.glsl")
        , framebufferWidth(1280)
        , framebufferHeight(800)
    {
        // Point light shadow maps
        glGenFramebuffers(1, &pointLightMapFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, pointLightMapFBO);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        window.AddResizeEventHandler([this](int width, int height) { this->OnFramebufferResize(width, height); });

        // VBO/VAO for drawing textures to fullscreen quad
        InitQuad();

        InitGBuffer();
    }

    void OnFramebufferResize(int width, int height)
    {
        framebufferWidth = width;
        framebufferHeight = height;

        // TODO -- do we need to release anything here?
        InitGBuffer();
    }

    void InitGBuffer();

    void DeferredRender(Scene& scene, const Camera& camera, std::function<void(ShaderProgram&)> drawGeometry);

    void RenderDirectionalLightShadowMap(Scene& scene);

    void AddWidgets();
    void ReloadShaders();

    void InitQuad();
    void DrawQuad(GLuint textureColorbuffer, vec2 min = vec2(0.0f), vec2 max = vec2(1.0f));

    static const int SHADOW_WIDTH = 1024;
    static const int SHADOW_HEIGHT = 1024;

private:

    int framebufferWidth;
    int framebufferHeight;

    bool drawShadowDepth;
    bool drawGBuffer;
    bool drawHDRBuffer;
    bool renderDirectionalShadowMap;
    bool renderPointShadowMaps;

    ShadowMap shadowMap;

    ShaderProgram pointLightShadowShader;
    GLuint pointLightMapFBO;

    ShaderProgram fullscreenShader;
    GLuint quadVBO;
    GLuint quadVAO;

    ShaderProgram geometryPassShader;
    ShaderProgram deferredShadingPassShader;
    GLuint gBufferFBO;
    GLuint gPositionBuffer;
    GLuint gNormalBuffer;
    GLuint gAlbedoSpecBuffer;
    GLuint gDepthRenderBuffer;

    ShaderProgram ssaoShader;
    GLuint ssaoFBO;
    GLuint ssaoColorBuffer;
    std::vector<glm::vec3> ssaoKernel;
    std::vector<glm::vec3> ssaoNoise;
    unsigned int noiseTexture;

    ShaderProgram ssaoBlurShader;
    GLuint ssaoBlurFBO;
    GLuint ssaoBlurColorBuffer;

    bool ssaoEnabled;
    int ssaoKernelSize;
    float ssaoRadius;
    float ssaoBias;

    ShaderProgram hdrShader;
    GLuint hdrFBO;
    GLuint hdrColorBuffer;

    ShaderProgram postShader;
    GLuint postFBO;
    GLuint postColorBuffer;

};
