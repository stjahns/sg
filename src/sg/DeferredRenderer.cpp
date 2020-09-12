#include <sstream>
#include <random>

#include "DeferredRenderer.h"

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void DeferredRenderer::ReloadShaders()
{
    fullscreenShader.Reload();
    geometryPassShader.Reload();
    deferredShadingPassShader.Reload();
    hdrShader.Reload();
    postShader.Reload();
}

void DeferredRenderer::AddWidgets()
{
    ImGui::Checkbox("Draw G Buffer", &drawGBuffer);
    ImGui::Checkbox("Draw HDR Buffer", &drawHDRBuffer);

    if (ImGui::Button("Reload Deferred Shaders"))
    {
        ReloadShaders();
    }

    if (ImGui::CollapsingHeader("SSAO"))
    {
        ImGui::Checkbox("Enabled", &ssaoEnabled);
        ImGui::DragFloat("Radius", &ssaoRadius, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Bias", &ssaoBias, 0.001f, 0.0f, 1.0f);
        ImGui::DragInt("Kernel Size", &ssaoKernelSize, 1, 1, 64);
    }

    if (ImGui::CollapsingHeader("Shadow"))
    {
        ImGui::Checkbox("Directional Map", &renderDirectionalShadowMap);
        ImGui::Checkbox("Point Maps", &renderPointShadowMaps);
        ImGui::Checkbox("Draw Depth", &drawShadowDepth);
        ImGui::DragFloat("Near Plane", &shadowMap.NEAR_PLANE, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Far Plane", &shadowMap.FAR_PLANE, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Ortho Width", &shadowMap.ORTHO_WIDTH, 0.1f, 0.0f, 100.0f);
    }
}

void DeferredRenderer::InitGBuffer()
{
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // - position color buffer
    glGenTextures(1, &gPositionBuffer);
    glBindTexture(GL_TEXTURE_2D, gPositionBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, framebufferWidth, framebufferHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionBuffer, 0);

    // - normal color buffer
    glGenTextures(1, &gNormalBuffer);
    glBindTexture(GL_TEXTURE_2D, gNormalBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, framebufferWidth, framebufferHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalBuffer, 0);

    // - color + specular color buffer
    glGenTextures(1, &gAlbedoSpecBuffer);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpecBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpecBuffer, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &gDepthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebufferWidth, framebufferHeight);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gDepthRenderBuffer);

    // Setup HDR framebuffer
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glGenTextures(1, &hdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorBuffer, 0);
    glDrawBuffers(1, attachments);

    // Setup post-processing framebuffer
    glGenFramebuffers(1, &postFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postFBO);

    glGenTextures(1, &postColorBuffer);
    glBindTexture(GL_TEXTURE_2D, postColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferWidth, framebufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postColorBuffer, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDrawBuffers(1, attachments);

    // Setup ssao framebuffer
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, framebufferWidth, framebufferHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    glDrawBuffers(1, attachments);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

    glGenTextures(1, &ssaoBlurColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, framebufferWidth, framebufferHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurColorBuffer, 0);
    glDrawBuffers(1, attachments);
    glBindTexture(GL_TEXTURE_2D, 0);

    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = (float)i / 64.0;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f);
        ssaoNoise.push_back(noise);
    }

    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glBindRenderbuffer(GL_RENDERBUFFER, gDepthRenderBuffer);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, hdrFBO);  

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRenderer::DeferredRender(Scene& scene, const Camera& camera, Model& model) // TODO -- split this up
{
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    geometryPassShader.Use();

    geometryPassShader.SetUniform("view", camera.GetViewMatrix());
    geometryPassShader.SetUniform("projection", camera.GetProjection());

    geometryPassShader.SetUniformi("material.diffuse", 0);
    geometryPassShader.SetUniformi("material.normal", 2);

    if (model.IsLoaded() && !model.IsBound())
    {
        model.Bind();
    }

    if (model.IsBound())
    {
        mat4 mvp = camera.GetProjection() * camera.GetViewMatrix() * model.GetTransform();
        geometryPassShader.SetUniform("model", model.GetTransform());
        geometryPassShader.SetUniform("MVP", mvp);
        model.Draw();
        glCheckError();
    }

    if (ssaoEnabled)
    {
        // SSAO PASS
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glCheckError();

        ssaoShader.Use();

        ssaoShader.SetUniformi("gPositionBuffer", 0);
        ssaoShader.SetUniformi("gNormalBuffer", 1);
        ssaoShader.SetUniformi("texNoise", 2);
        ssaoShader.SetUniform("view", camera.GetViewMatrix());
        ssaoShader.SetUniform("projection", camera.GetProjection());

        ssaoShader.SetUniformi("kernelSize", ssaoKernelSize);
        ssaoShader.SetUniform("radius", ssaoRadius);
        ssaoShader.SetUniform("bias", ssaoBias);

        for (int i = 0; i < 64; ++i)
        {
            std::ostringstream id;
            id << "samples[" << i << "]";
            ssaoShader.SetUniform(id.str().c_str(), ssaoKernel[i]);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionBuffer);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormalBuffer);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glCheckError();

        // Blur pass
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glCheckError();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

        ssaoBlurShader.Use();

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glCheckError();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Deferred shading pass

#if HDR_FRAMEBUFFER
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glCheckError();
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, postFBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glCheckError();
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    deferredShadingPassShader.Use();

    // Set texture units to G buffer outputs
    deferredShadingPassShader.SetUniformi("gPositionBuffer", 0);
    deferredShadingPassShader.SetUniformi("gNormalBuffer", 1);
    deferredShadingPassShader.SetUniformi("gAlbedoSpecBuffer", 2);
    deferredShadingPassShader.SetUniformi("ssaoBuffer", 8);

    deferredShadingPassShader.SetUniform("uViewPos", camera.GetPosition());

    deferredShadingPassShader.SetUniform("uDirectionalLight.ambient", scene.GetDirectionalLight().ambient);
    deferredShadingPassShader.SetUniform("uDirectionalLight.diffuse", scene.GetDirectionalLight().diffuse);
    deferredShadingPassShader.SetUniform("uDirectionalLight.specular", scene.GetDirectionalLight().specular);
    deferredShadingPassShader.SetUniform("uDirectionalLight.direction", scene.GetDirectionalLight().direction);

    deferredShadingPassShader.SetUniform("numPointLights", scene.GetNumPointLights());

    for (int i = 0; i < Scene::maxPointLights; ++i)
    {
        PointLight& light = scene.GetPointLights()[i];

        std::ostringstream id;
        id << "pointLights[" << i << "]";

        glActiveTexture(GL_TEXTURE4 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light.depthCubemap);

        deferredShadingPassShader.SetUniform((id.str() + ".position").c_str(), light.position);
        deferredShadingPassShader.SetUniform((id.str() + ".ambient").c_str(), light.ambient);
        deferredShadingPassShader.SetUniform((id.str() + ".diffuse").c_str(), light.diffuse);
        deferredShadingPassShader.SetUniform((id.str() + ".specular").c_str(), light.specular);
        deferredShadingPassShader.SetUniform((id.str() + ".constant").c_str(), light.constant);
        deferredShadingPassShader.SetUniform((id.str() + ".linear").c_str(), light.linear);
        deferredShadingPassShader.SetUniform((id.str() + ".quadratic").c_str(), light.quadratic);
        deferredShadingPassShader.SetUniform((id.str() + ".far_plane").c_str(), 25.0f); // umm...
        deferredShadingPassShader.SetUniformi((id.str() + ".depthMap").c_str(), 4 + i);
    }

    glBindVertexArray(quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPositionBuffer);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormalBuffer);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpecBuffer);

    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);

    // TODO -- unbind/cleanup stuff!

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glCheckError();

#if HDR_FRAMEBUFFER
    // Render hdr colorbuffer to default framebuffer
    hdrShader.Use();
    hdrShader.SetUniformi("hdrBuffer", 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glCheckError();
#endif

    // Render post-processsing pass to default framebuffer
    postShader.Use();
    postShader.SetUniformi("colorBuffer", 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postColorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glCheckError();

    // Copy depth information from GBuffer to default framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, framebufferWidth, framebufferHeight, 0, 0, framebufferWidth, framebufferHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    if (drawGBuffer)
    {
        DrawQuad(gPositionBuffer, vec2(-1.0, 0.0), vec2(0.0, 1.0));
        DrawQuad(gNormalBuffer, vec2(0.0, 0.0), vec2(1.0, 1.0));
        DrawQuad(gAlbedoSpecBuffer, vec2(-1.0, -1.0), vec2(0.0, 0.0));
    }

    if (drawHDRBuffer)
    {
        //DrawQuad(hdrColorBuffer, vec2(0.0, -1.0), vec2(1.0, 0.0));
        //DrawQuad(postColorBuffer, vec2(0.0, -1.0), vec2(1.0, 0.0));
        DrawQuad(ssaoBlurColorBuffer, vec2(0.0, -1.0), vec2(1.0, 0.0));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void DeferredRenderer::RenderDirectionalLightShadowMap(Scene& scene)
{
    if (!renderDirectionalShadowMap)
    {
        return;
    }

    // TODO -- don't need to regenerate this if the scene doesn't change... can save us some grief

    shadowMap.PreRender(scene.GetDirectionalLight().direction);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        /* fixme
        for (Model* model : scene.GetModels())
        {
            shadowMap.SetModel(model->GetTransform());
            model->Draw();
            glCheckError();
        }
        */
    }

    shadowMap.PostRender();

    // TODO -- this must update...
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    if (drawShadowDepth)
    {
        DrawQuad(shadowMap.GetDepthMap());
    }
}

void DeferredRenderer::InitQuad()
{
    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // screen quad VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void DeferredRenderer::DrawQuad(GLuint textureColorbuffer, vec2 min, vec2 max)
{
    fullscreenShader.Use();
    fullscreenShader.SetUniform("min", min);
    fullscreenShader.SetUniform("max", max);
    glBindVertexArray(quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}