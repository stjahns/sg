#include <sstream>
#include <random>

#include "Renderer.h"

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void Renderer::AddWidgets()
{
    ImGui::Checkbox("Draw G Buffer", &drawGBuffer);
    ImGui::Checkbox("Draw HDR Buffer", &drawHDRBuffer);

    if (ImGui::Button("Reload Deferred Shaders"))
    {
        fullscreenShader.Reload();
        geometryPassShader.Reload();
        deferredShadingPassShader.Reload();
        hdrShader.Reload();
        postShader.Reload();
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

void Renderer::InitGBuffer()
{
    glGenFramebuffers(1, &gBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // - position color buffer
    glGenTextures(1, &gPositionBuffer);
    glBindTexture(GL_TEXTURE_2D, gPositionBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionBuffer, 0);

    // - normal color buffer
    glGenTextures(1, &gNormalBuffer);
    glBindTexture(GL_TEXTURE_2D, gNormalBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalBuffer, 0);

    // - color + specular color buffer
    glGenTextures(1, &gAlbedoSpecBuffer);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpecBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpecBuffer, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &gDepthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gDepthRenderBuffer);

    // Setup HDR framebuffer
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glGenTextures(1, &hdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorBuffer, 0);
    glDrawBuffers(1, attachments);

    // Setup post-processing framebuffer
    glGenFramebuffers(1, &postFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postFBO);

    glGenTextures(1, &postColorBuffer);
    glBindTexture(GL_TEXTURE_2D, postColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    glDrawBuffers(1, attachments);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

    glGenTextures(1, &ssaoBlurColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
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

void Renderer::DeferredRender(Scene& scene, const Camera& camera, Model& model) // TODO -- split this up
{
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

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
    glBlitFramebuffer(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

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


void Renderer::ForwardRender(Scene& scene, const Camera& camera, ShaderProgram& activeShader)
{
    PointLight* pointLights = scene.GetPointLights();
    SpotLight* spotLights = scene.GetSpotLights();

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjection();

    glActiveTexture(GL_TEXTURE3);

    if (renderDirectionalShadowMap)
    {
        glBindTexture(GL_TEXTURE_2D, shadowMap.GetDepthMap());
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // must init all the samplers uniquely or we will get errors if they are left to default (0)
    for (int i = 0; i < Scene::maxPointLights; ++i)
    {
        glActiveTexture(GL_TEXTURE4 + i);

        if (renderPointShadowMaps)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, pointLights[i].depthCubemap);
        }
        else
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }

        activeShader.Use();

        std::ostringstream id;
        id << "pointLights[" << i << "]";

        activeShader.SetUniformi((id.str() + ".depthMap").c_str(), 4 + i);
        activeShader.SetUniform((id.str() + ".far_plane").c_str(), 25.0f); // fixme...
    }

    DirectionalLight& directionalLight = scene.GetDirectionalLight();


    /*
    for (Model* model : scene.GetModels())
    {
        PrepareForwardRenderModelBatch(scene, *model, activeShader);
        ForwardRenderModel(*model, activeShader, model->GetTransform());
    }
    */
}

void Renderer::PrepareForwardRenderModelBatch(Scene& scene, const Camera& camera, Model& model, ShaderProgram& activeShader)
{
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjection();
    DirectionalLight& directionalLight = scene.GetDirectionalLight();
    PointLight* pointLights = scene.GetPointLights();
    SpotLight* spotLights = scene.GetSpotLights();

    if (model.IsLoaded() && !model.IsBound())
    {
        model.Bind();
    }

    if (model.IsBound())
    {
        activeShader.Use();

        activeShader.SetUniformi("material.diffuse", 0);
        activeShader.SetUniformi("material.specular", 1);
        activeShader.SetUniformi("material.normal", 2);

        activeShader.SetUniformi("shadowMap", 3);
        activeShader.SetUniformi("depthMap", 4); // test...
        //activeShader.SetUniform("lightSpaceMatrix", shadowMap.GetLightSpaceMatrix(directionalLight.direction));

        activeShader.SetUniform("view", view);
        activeShader.SetUniform("projection", projection);

        //activeShader.SetUniform("objectColor", vec3(1.0f, 1.0f, 1.0f));
        //activeShader.SetUniform("material.diffuse", vec3(1.0f, 1.0f, 1.0f));
        //activeShader.SetUniform("material.ambient", vec3(1.0f, 1.0f, 1.0f));
        //activeShader.SetUniform("material.specular", vec3(1.0f, 1.0f, 1.0f));

        activeShader.SetUniform("directionalLight.direction", directionalLight.direction);
        // TODO - Fix point shadow frustums
        activeShader.SetUniform("directionalLight.ambient", directionalLight.ambient);
        activeShader.SetUniform("directionalLight.diffuse", directionalLight.diffuse);
        activeShader.SetUniform("directionalLight.specular", directionalLight.specular);

        activeShader.SetUniform("numPointLights", scene.GetNumPointLights());

        for (int i = 0; i < scene.GetNumPointLights(); ++i)
        {
            std::ostringstream id;
            id << "pointLights[" << i << "]";

            PointLight& light = pointLights[i];
            activeShader.SetUniform((id.str() + ".position").c_str(), pointLights[i].position);
            activeShader.SetUniform((id.str() + ".ambient").c_str(), pointLights[i].ambient);
            activeShader.SetUniform((id.str() + ".diffuse").c_str(), pointLights[i].diffuse);
            activeShader.SetUniform((id.str() + ".specular").c_str(), pointLights[i].specular);
            activeShader.SetUniform((id.str() + ".constant").c_str(), pointLights[i].constant);
            activeShader.SetUniform((id.str() + ".linear").c_str(), pointLights[i].linear);
            activeShader.SetUniform((id.str() + ".quadratic").c_str(), pointLights[i].quadratic);
        }

        activeShader.SetUniform("numSpotLights", scene.GetNumSpotLights());

        for (int i = 0; i < scene.GetNumSpotLights(); ++i)
        {
            std::ostringstream id;
            id << "spotLights[" << i << "]";

            SpotLight& light = spotLights[i];
            activeShader.SetUniform((id.str() + ".position").c_str(), spotLights[i].position);
            activeShader.SetUniform((id.str() + ".direction").c_str(), spotLights[i].direction);
            activeShader.SetUniform((id.str() + ".ambient").c_str(), spotLights[i].ambient);
            activeShader.SetUniform((id.str() + ".diffuse").c_str(), spotLights[i].diffuse);
            activeShader.SetUniform((id.str() + ".specular").c_str(), spotLights[i].specular);
            activeShader.SetUniform((id.str() + ".constant").c_str(), spotLights[i].constant);
            activeShader.SetUniform((id.str() + ".linear").c_str(), spotLights[i].linear);
            activeShader.SetUniform((id.str() + ".quadratic").c_str(), spotLights[i].quadratic);
            activeShader.SetUniform((id.str() + ".cutoff").c_str(), spotLights[i].cutOff);
            activeShader.SetUniform((id.str() + ".outercutoff").c_str(), spotLights[i].outerCutOff);
        }

        activeShader.SetUniform("viewPos", camera.GetPosition());
    }
}

void Renderer::ForwardRenderModel(Model& model, ShaderProgram& activeShader, const mat4& transform)
{
    activeShader.SetUniform("model", transform);
    model.Draw(&activeShader);
}

void Renderer::RenderPointLightShadowMaps(Scene& scene)
{
    if (!renderPointShadowMaps)
    {
        return;
    }

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, pointLightMapFBO);
    glCheckError();

    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    float nearPlane = 0.0f;
    float farPlane = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);

    PointLight* pointLights = scene.GetPointLights();
    SpotLight* spotLights = scene.GetSpotLights();

    for (int i = 0; i < scene.GetNumPointLights(); ++i)
    {
        std::vector<glm::mat4> shadowTransforms;

        // view matrices looking in each direction
        // TODO -- can probably precompute these and just update the translations...
        shadowTransforms.push_back(shadowProj * glm::lookAt(pointLights[i].position, pointLights[i].position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(pointLights[i].position, pointLights[i].position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(pointLights[i].position, pointLights[i].position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(pointLights[i].position, pointLights[i].position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(pointLights[i].position, pointLights[i].position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(pointLights[i].position, pointLights[i].position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

        for (int j = 0; j < 6; ++j)
        {
            GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + j;
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, pointLights[i].depthCubemap, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glClear(GL_DEPTH_BUFFER_BIT);
            glCheckError();

            pointLightShadowShader.Use();

            pointLightShadowShader.SetUniform("shadowMatrix", shadowTransforms[j]);
            pointLightShadowShader.SetUniform("far_plane", farPlane);
            pointLightShadowShader.SetUniform("lightPos", pointLights[i].position);

            /* FIXME
            for (Model* model : scene.GetModels())
            {
                pointLightShadowShader.SetUniform("model", model->GetTransform());
                model->Draw();
                glCheckError();
            }
            */
        }
    }

    // TODO -- this must update if we resize window..
    glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderDirectionalLightShadowMap(Scene& scene)
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
    glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    if (drawShadowDepth)
    {
        DrawQuad(shadowMap.GetDepthMap());
    }
}

void Renderer::InitQuad()
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

void Renderer::DrawQuad(GLuint textureColorbuffer, vec2 min, vec2 max)
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