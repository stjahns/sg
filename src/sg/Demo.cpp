#include <sstream>
#include <glm/gtc/random.hpp>

#include "LineRenderer.h"
#include "Asset.h"
#include "anim/AssimpSkeletonLoader.h"
#include "anim/AssimpClipLoader.h"

#include <tiny_gltf.h>
#include "Demo.h"

struct EntityTransform
{
    vec3 position;
    quat orientation;

    mat4 ToMat4()
    {
        mat4 matrix = mat4_cast(orientation);
        matrix[3] = vec4(position, 1.0f);
        return matrix;
    }
};

struct EntityModel
{
    int modelIndex;
};

struct AnimGraph
{
    AnimGraph(std::unique_ptr<AnimationNode> node) : rootNode(std::move(node))
    {
    }

    std::unique_ptr<AnimationNode> rootNode;
};

typedef vec3 Velocity;

Demo::Demo(GLFWWindow& window) 
    : window(window)
    , shaderProgram("shaders/skinned.vs.glsl", "shaders/mixamo.fs.glsl")
    , drawWireframe(false)
    , drawSkeleton(false)
    , p1(0.0f)
    , time(glfwGetTime())
    , timescale(1.0f)
{
    window.AddMouseEventHandler([&](MouseEvent e) { scene.GetCamera().OnMouseEvent(e); });
    LoadBiped();
    //LoadFox();
}


void Demo::LoadSkeletonFromFile(std::string filename, Skeleton& skeleton)
{
    const std::string path = GetAssetPath(filename);
    Assimp::Importer importer;
    const aiScene* assimpScene = importer.ReadFile(path, 0);
    if (assimpScene)
    {
        LoadSkeleton(*assimpScene, skeleton);
    }
}

void Demo::LoadClipFromFile(std::string filename, Skeleton& skeleton, int clipIndex, std::string idPrefix)
{
    clips.emplace_back();
    AnimationClip& clip = clips.back();

    const std::string path = GetAssetPath(filename);

    Assimp::Importer importer;
    const aiScene* assimpScene = importer.ReadFile(path, 0);
    if (assimpScene)
    {
        LoadClip(*assimpScene, skeleton, clip, clipIndex, idPrefix);
    }

    if (skeleton.rootMotionBone.IsValid())
    {
        TranslationChannel rootMotionTranslation(skeleton.rootMotionBone);
        RotationChannel rootMotionRotation(skeleton.rootMotionBone);

        clip.ComputeAdditiveRootMotion(rootMotionTranslation, rootMotionRotation);
        clip.AddRotationChannel(rootMotionRotation);
        clip.AddTranslationChannel(rootMotionTranslation);
    }
}

void Demo::LoadBiped()
{
    // TODO -- put em on a floor
    // TODO -- add better lighting! (deferred render w/ color material?)
    // TODO -- cleanup!

    Model& model = models.emplace_back("Models/ybot/ybot.gltf");
    model.Load();

    scene.GetDirectionalLight().ambient = vec3(0.4f, 0.4f, 0.4f);
    LoadSkeletonFromFile("Models/ybot/ybot.gltf", skeleton);

    skeleton.AddRootMotionBone();

    // TODO something less dumb
    for (int i = 0; i < skeleton.bones.size(); ++i)
    {
        std::ostringstream id;
        id << "skinMatrix[" << i << "]";
        skinUniformIds.push_back(id.str());
    }

    LoadClipFromFile("Clips/armada.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/armada to esquiva.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/ginga sideways to au.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/au.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/au to role.fbx", skeleton, 0, "mixamorig_");
    /*
    LoadClipFromFile("Clips/esquiva 1.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/esquiva 2.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/esquiva 3.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/esquiva 4.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/esquiva 5.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/ginga backward.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/ginga forward.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/ginga sideways to au.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/au.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/au to role.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/pontera.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/martelo do chau.fbx", skeleton, 0, "mixamorig_");
    */

    float spacing = 100.0f;

    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            auto entity = entityRegistry.create();

            float heading = glm::linearRand(0.0f, two_pi<float>());
            entityRegistry.emplace<EntityTransform>(entity, vec3(-i * spacing, 0.0f, -j * spacing), quat(vec3(0.0f, heading, 0.0f)));

            entityRegistry.emplace<Pose>(entity, skeleton.bindPose);
            entityRegistry.emplace<EntityModel>(entity, 0);

            auto stateMachineNode = std::make_unique<StateMachineNode>();

            const int ParameterId = 1;
            const int TriggerValue = 1;

            int rand = glm::linearRand(0, int(clips.size() - 1));

            for (int k = 0; k < clips.size(); ++k)
            {
                int clipIndex = (k + rand) % clips.size();

                auto clipNode = std::make_unique<ClipNode>(clips[clipIndex]);
                clipNode->AddPhaseTrigger({ 0.80f, ParameterId, TriggerValue });
                stateMachineNode->AddState(std::move(clipNode));

                {
                    Condition condition{ ParameterId, TriggerValue };
                    Transition transition{ clipIndex, (clipIndex + 1) % clips.size(), 0.50f, condition };
                    stateMachineNode->AddTransition(transition);
                }
            }

            stateMachineNode->Update(glm::linearRand(0.0f, 10.0f), Parameters());

            entityRegistry.emplace<AnimGraph>(entity, std::move(stateMachineNode));

        }
    }
}

void Demo::LoadFox()
{
    timescale = 1000.0f;


    const std::string filename = "Models/Fox/gLTF/Fox.gltf";
    const std::string assetPath = GetAssetPath(filename);

    LoadSkeletonFromFile(filename, skeleton);
    LoadClipFromFile(filename, skeleton, 0);
    LoadClipFromFile(filename, skeleton, 1);
    LoadClipFromFile(filename, skeleton, 2);

    Model& model = models.emplace_back(filename.c_str());
    model.Load();

    float spacing = 100.0f;

    for (int i = 0; i < skeleton.bones.size(); ++i)
    {
        std::ostringstream id;
        id << "skinMatrix[" << i << "]";
        skinUniformIds.push_back(id.str());
    }

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            auto entity = entityRegistry.create();

            entityRegistry.emplace<EntityTransform>(entity, vec3(-i * spacing, 0.0f, -j * spacing), quat());
            entityRegistry.emplace<EntityModel>(entity, 0);
            entityRegistry.emplace<Pose>(entity, skeleton.bindPose);

            auto stateMachineNode = std::make_unique<StateMachineNode>();

            const int ParameterId = 1;
            const int TriggerValue = 1;

            for (int i = 0; i < clips.size(); ++i)
            {
                auto clipNode = std::make_unique<ClipNode>(clips[i]);
                clipNode->AddPhaseTrigger({ 0.95f, ParameterId, TriggerValue });
                stateMachineNode->AddState(std::move(clipNode));

                {
                    Condition condition{ ParameterId, TriggerValue };
                    Transition transition{ i, (i + 1) % clips.size(), 1000.0f, condition };
                    stateMachineNode->AddTransition(transition);
                }
            }

            stateMachineNode->Update(glm::linearRand(0.0f, 2000.0f), Parameters{ { 1, glm::linearRand(0, 2) } });

            entityRegistry.emplace<AnimGraph>(entity, std::move(stateMachineNode));
        }
    }
}

void Demo::Run()
{
    while (!window.ShouldClose())
    {
        window.Update();

        if (window.GetKey(GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            window.Close();
        }

        if (window.GetKey(GLFW_KEY_R) == GLFW_PRESS)
        {
            shaderProgram.Reload();
        }

        double deltaTime = glfwGetTime() - time;
        time += deltaTime;

        Update(deltaTime * timescale);

        shaderProgram.Use();

        ImVec4 clear_color = ImColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Camera& camera = scene.GetCamera();
		camera.Update(window);
		mat4 view = camera.GetViewMatrix();

		glEnable(GL_DEPTH_TEST);

		WindowSize windowSize = window.GetWindowSize();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)windowSize.width / (float)windowSize.height, 0.01f, 10000.0f);
		camera.SetProjection(projection);

        if (drawWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		renderer.ForwardRender(scene, shaderProgram);

        ForwardRenderModels();

        if (drawWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		scene.DrawLights();

        lineRenderer.AddLine(vec3(0), vec3(1, 0, 0), vec4(1, 0, 0, 1), vec4(1, 0, 0, 0));
        lineRenderer.AddLine(vec3(0), vec3(0, 1, 0), vec4(0, 1, 0, 1), vec4(0, 1, 0, 0));
        lineRenderer.AddLine(vec3(0), vec3(0, 0, 1), vec4(0, 0, 1, 1), vec4(0, 0, 1, 0));
        lineRenderer.Render(view, projection);

        Widgets();
        ImGui::Render();

        window.SwapBuffers();
    }
}

void Demo::ForwardRenderModels()
{
    auto view = entityRegistry.view<EntityTransform, EntityModel, Pose>();

    if (models.size() == 1)
    {
        // TODO figure out how to handle batching multple models
        renderer.PrepareForwardRenderModelBatch(scene, models[0], shaderProgram);
    }

    std::vector<mat4> skinMatrices;
    skinMatrices.resize(skeleton.bones.size());

    for (auto entity : view)
    {
        auto& transform = view.get<EntityTransform>(entity);
        auto& entityModel = view.get<EntityModel>(entity);
        auto& pose = view.get<Pose>(entity);

        Model& model = models[entityModel.modelIndex];

        pose.ComputeObjectFromLocal(skeleton);

        for (int i = 0; i < skeleton.bones.size(); ++i)
        {
            skinMatrices[i] = pose.objectTransforms[i] * skeleton.bones[i].inverseBindPose;
        }

        shaderProgram.Use();

        for (int i = 0; i < skeleton.bones.size(); ++i)
        {
            shaderProgram.SetUniform(skinUniformIds[i].c_str(), skinMatrices[i]);
        }

        renderer.ForwardRenderModel(model, shaderProgram, transform.ToMat4());
    }
}

void Demo::Update(float deltaTime)
{
    auto view = entityRegistry.view<AnimGraph, Pose, EntityTransform>();

    AnimationPose pose;
    
    pose.Resize(skeleton.bones.size()); // TODO skeleton.Size()
 
    for (auto entity : view)
    {
        auto& animGraph = view.get<AnimGraph>(entity);
        animGraph.rootNode->Update(deltaTime, Parameters{ { 1, p1 } });

        animGraph.rootNode->Evaluate(pose);

        auto& skeletonPose = view.get<Pose>(entity);
        auto& transform = view.get<EntityTransform>(entity);

        if (skeleton.rootMotionBone.IsValid())
        {
            vec3 rootTranslation;
            pose.GetTranslation(0, rootTranslation);
            rootTranslation.x = 0.0f;
            rootTranslation.z = 0.0f;
            pose.SetTranslation(0, rootTranslation);
        }

        skeleton.ApplyAnimationPose(pose, skeletonPose);

        skeletonPose.ComputeObjectFromLocal(skeleton);

        if (skeleton.rootMotionBone.IsValid())
        {
            vec3 rootMotion;
            pose.GetTranslation(skeleton.rootMotionBone, rootMotion);

            rootMotion.y = 0.0f;
            transform.position += transform.orientation * (rootMotion * deltaTime);
        }

        if (drawSkeleton)
        {
            mat4 modelTransform = transform.ToMat4();
            lineRenderer.AddPose(modelTransform, skeleton, skeletonPose, vec4(1));
        }
    }
}

void Demo::Widgets()
{
    ImGui::Begin("Demo");

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::DragFloat("Timescale", &timescale, 0.01, 0.0, 1000.0);

    if (ImGui::CollapsingHeader("Render", "RENDER", true, true))
    {
        ImGui::Checkbox("Wireframe", &drawWireframe);
        ImGui::Checkbox("Skeleton", &drawSkeleton);
    }

    if (ImGui::CollapsingHeader("Parameters", "PARAMETERS", true, true))
    {
        ImGui::DragInt("P1", &p1, 1.0f, 0, 1);
    }

    scene.AddWidgets();

    ImGui::End();
}
