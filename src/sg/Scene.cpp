#include <sstream>

#include "Scene.h"

#include "LineRenderer.h"
#include "Asset.h"
#include "anim/AssimpSkeletonLoader.h"
#include "anim/AssimpClipLoader.h"

Scene::Scene() : lightModel("models/glTF/box/box.gltf")
, numPointLights(0)
, numSpotLights(0)
, lightShader("shaders/mvp.vs.glsl", "shaders/white.fs.glsl")
{
    lightModel.Load();

    Assimp::Importer importer;
    //const aiScene* scene = importer.ReadFile(GetAssetPath("Models/RiggedSimple/gLTF/RiggedSimple.gltf"), 0);
    const aiScene* scene = importer.ReadFile(GetAssetPath("Models/Fox/gLTF/Fox.gltf"), 0);
    if (scene)
    {
        LoadSkeleton(*scene, skeleton);
        LoadClip(*scene, skeleton, clip);
        tick = 0.0f;
    }
}

void Scene::Update(LineRenderer& lineRenderer)
{
    clip.EvaulatePose(skeleton, tick, skeleton.bindPose);

    tick += 10.0f;
    tick = fmodf(tick, clip.duration);

    skeleton.bindPose.ComputeObjectFromLocal(skeleton);

    lineRenderer.AddPose(skeleton, skeleton.bindPose, vec4(1));
}

void Scene::AddWidgets()
{
    camera.Widgets();

    ImGui::PushID("DirectionalLight");
    if (ImGui::CollapsingHeader("Directional Light"))
    {
        ImGui::DragFloat3("Direction", (float *)&directionalLight.direction, 0.01, -1, 1);
        ImGui::DragFloat3("Diffuse", (float *)&directionalLight.diffuse, 0.01, 0.0, 100.0);
        ImGui::DragFloat3("Specular", (float *)&directionalLight.specular, 0.01, 0.0, 1.0);
        ImGui::DragFloat3("Ambient", (float *)&directionalLight.ambient, 0.01, 0.0, 1.0);
    }
    ImGui::PopID();

    if (ImGui::Button("Add Point Light"))
    {
        if (numPointLights < maxPointLights)
        {
            ++numPointLights;
        }
    }

    if (ImGui::Button("Remove Point Light"))
    {
        if (numPointLights > 0)
        {
            --numPointLights;
        }
    }

    for (int i = 0; i < numPointLights; ++i)
    {
        std::ostringstream label;
        label << "Point Light " << i;
        PointLight &light = pointLights[i];
        ImGui::PushID(i);
        if (ImGui::CollapsingHeader(label.str().c_str()))
        {
            ImGui::DragFloat3("Position", (float *)&light.position, 0.1, -100, 100);
            ImGui::DragFloat3("Diffuse", (float *)&light.diffuse, 0.01, 0.0, 100.0);
            ImGui::DragFloat3("Specular", (float *)&light.specular, 0.01, 0.0, 1.0);
            ImGui::DragFloat3("Ambient", (float *)&light.ambient, 0.01, 0.0, 1.0);
            ImGui::DragFloat("Constant", &light.constant, 0.01, 0.0, 100.0);
            ImGui::DragFloat("Linear", &light.linear, 0.01, 0.0, 100.0);
            ImGui::DragFloat("Quadratic", &light.quadratic, 0.01, 0.0, 100.0);
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Add Spot Light"))
    {
        if (numSpotLights < maxSpotLights)
        {
            ++numSpotLights;
        }
    }

    if (ImGui::Button("Remove Spot Light"))
    {
        if (numSpotLights > 0)
        {
            --numSpotLights;
        }
    }

    for (int i = 0; i < numSpotLights; ++i)
    {
        std::ostringstream label;
        label << "Spot Light " << i;
        SpotLight &light = spotLights[i];
        ImGui::PushID(i);
        if (ImGui::CollapsingHeader(label.str().c_str()))
        {
            ImGui::DragFloat3("Position", (float *)&light.position, 0.01, -100, 100);
            ImGui::DragFloat3("Direction", (float *)&light.direction, 0.01, -100, 100);
            ImGui::DragFloat3("Diffuse", (float *)&light.diffuse, 0.01, 0.0, 1.0);
            ImGui::DragFloat3("Specular", (float *)&light.specular, 0.01, 0.0, 1.0);
            ImGui::DragFloat3("Ambient", (float *)&light.ambient, 0.01, 0.0, 1.0);
            ImGui::DragFloat("Constant", &light.constant, 0.01, 0.0, 100.0);
            ImGui::DragFloat("Linear", &light.linear, 0.01, 0.0, 100.0);
            ImGui::DragFloat("Quadratic", &light.quadratic, 0.01, 0.0, 100.0);
            ImGui::DragFloat("Cutoff", &light.cutOff, 0.01, 0.0, 1.0);
            ImGui::DragFloat("Outer Cutoff", &light.outerCutOff, 0.01, 0.0, 1.0);
        }
        ImGui::PopID();
    }

    if (ImGui::CollapsingHeader("Models"))
    {
        static char pathBuffer[1024];
        ImGui::InputText("Path", pathBuffer, sizeof(pathBuffer));

        if (ImGui::Button("Add"))
        {
            LoadModel(pathBuffer);
        }

        int modelId = 0;

        for (Model *model : models)
        {
			ImGui::PushID(modelId++);
			if (ImGui::CollapsingHeader(model->GetFilename()))
			{
				model->Widgets();
			}
			ImGui::PopID();
        }
    }
}

void Scene::DrawLights()
{
    if (lightModel.IsLoaded() && !lightModel.IsBound())
    {
        lightModel.Bind();
    }

    if (!lightModel.IsBound())
    {
        return;
    }

    mat4 view = camera.GetViewMatrix();
    mat4 projection = camera.GetProjection();

    for (int i = 0; i < numPointLights; ++i)
    {
        mat4 lightMatrix(1.0f);

        lightMatrix = translate(lightMatrix, pointLights[i].position);
        lightMatrix = scale(lightMatrix, vec3(lightScale));

        lightShader.Use();
        lightShader.SetUniform("model", lightMatrix);
        lightShader.SetUniform("view", view);
        lightShader.SetUniform("projection", projection);
        lightModel.Draw();
    }

    for (int i = 0; i < numSpotLights; ++i)
    {
        mat4 lightMatrix(1.0f);

        lightMatrix = translate(lightMatrix, spotLights[i].position);
        lightMatrix = scale(lightMatrix, vec3(lightScale));

        lightShader.Use();
        lightShader.SetUniform("model", lightMatrix);
        lightShader.SetUniform("view", view);
        lightShader.SetUniform("projection", projection);
        lightModel.Draw();
    }
}