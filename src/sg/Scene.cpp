#include <sstream>

#include "Scene.h"

#include "LineRenderer.h"
#include "Asset.h"
#include "anim/AssimpSkeletonLoader.h"
#include "anim/AssimpClipLoader.h"

#include <tiny_gltf.h>

Scene::Scene() : lightModel("models/glTF/box/box.gltf")
, numPointLights(0)
, numSpotLights(0)
, lightShader("shaders/mvp.vs.glsl", "shaders/white.fs.glsl")
, transitionTime(0.0f)
, p1(0)
{
    lightModel.Load();

    // TODO -- can we figure out how to load skeletons with assimp?

    Assimp::Importer importer;

    const std::string filename = "Models/Fox/gLTF/Fox.gltf";
    const std::string assetPath = GetAssetPath(filename);

    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;

    if (loader.LoadASCIIFromFile(&model, &err, &warn, assetPath))
    {
        skeleton.Load(model);
    }

    const aiScene* scene = importer.ReadFile(assetPath, 0);
    if (scene)
    {
        LoadClip(*scene, skeleton, clip1, 0);
        LoadClip(*scene, skeleton, clip2, 2);

        clipNode1 = std::make_unique<ClipNode>(clip1);
        clipNode2 = std::make_unique<ClipNode>(clip2);

        State state1{ *clipNode1 }, state2{ *clipNode2 };

        stateMachine = std::make_unique<StateMachineNode>();
        stateMachine->AddState(state1);
        stateMachine->AddState(state2);

        {
            Condition condition{ 1, 1 };
            Transition transition{ 0, 1, 1000.0f, condition };
            stateMachine->AddTransition(transition);
        }

        {
            Condition condition{ 1, 0 };
            Transition transition{ 1, 0, 1000.0f, condition };
            stateMachine->AddTransition(transition);
        }
    }

    LoadModel(filename.c_str());
}

// TODO - safer type for time (ms, s??)
void Scene::Update(LineRenderer& lineRenderer, ShaderProgram& shader, float deltaTime)
{
    if (stateMachine)
    {
        Parameters parameters{ {1, p1 } };
        stateMachine->Update(deltaTime, parameters);

        AnimationPose pose;
        stateMachine->Evaluate(pose);

        skeleton.ApplyAnimationPose(pose, skeleton.currentPose);
    }

    skeleton.currentPose.ComputeObjectFromLocal(skeleton);

    shader.Use();

    for (int i = 0; i < skeleton.bones.size(); ++i)
    {
        mat4 skinMatrix = skeleton.currentPose.objectTransforms[i] * skeleton.bones[i].inverseBindPose;

        std::ostringstream id;
        id << "skinMatrix[" << i << "]";
        shader.SetUniform((id.str()).c_str(), skinMatrix);
    }

    lineRenderer.AddPose(skeleton, skeleton.currentPose, vec4(1));
}

void Scene::AddWidgets()
{
    if (ImGui::CollapsingHeader("Animation"))
    {
        ImGui::DragInt("P1", &p1, 1.0f, 0, 1);
        ImGui::DragFloat("Transition Time", &transitionTime, 0.01, 0.0f, 5.0f); // TODO -- allow controlling transition durations via param?
    }

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