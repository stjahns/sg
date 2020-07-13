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
    quat orientation; // wonder if splitting these up is useful sometimes? (eg broad phase culling?)

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
    , shaderProgram("shaders/skinned.vs.glsl", "shaders/fox.fs.glsl")
    , drawWireframe(false)
    , p1(0.0f)
    , time(glfwGetTime())
    , timescale(1.0f)
{
    window.AddMouseEventHandler([&](MouseEvent e) { scene.GetCamera().OnMouseEvent(e); });

    // TODO -- something to switch between these
    LoadBiped();
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

AnimationClip& Demo::LoadClipFromFile(std::string filename, Skeleton& skeleton, int clipIndex, std::string idPrefix)
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
    
    return clip;
}

void Demo::LoadBiped()
{
    LoadSkeletonFromFile("Models/ybot/ybot.gltf", skeleton);

    skeleton.AddRootMotionBone();

    LoadClipFromFile("Clips/armada.fbx", skeleton, 0, "mixamorig_");
    LoadClipFromFile("Clips/armada to esquiva.fbx", skeleton, 0, "mixamorig_");

    auto entity = entityRegistry.create();

    entityRegistry.emplace<EntityTransform>(entity, vec3(), quat());
    entityRegistry.emplace<Pose>(entity, skeleton.bindPose);

    auto clipNode1 = std::make_unique<ClipNode>(clips[0]);
    auto clipNode2 = std::make_unique<ClipNode>(clips[1]);

    auto stateMachineNode = std::make_unique<StateMachineNode>();

    stateMachineNode->AddState(std::move(clipNode1));
    stateMachineNode->AddState(std::move(clipNode2));

    {
        Condition condition{ 1, 0 };
        Transition transition{ 1, 0, 1.0f, condition };
        stateMachineNode->AddTransition(transition);
    }

    {
        Condition condition{ 1, 1 };
        Transition transition{ 0, 1, 1.0f, condition };
        stateMachineNode->AddTransition(transition);
    }

    entityRegistry.emplace<AnimGraph>(entity, std::move(stateMachineNode));
}

void Demo::LoadFox()
{
    const std::string filename = "Models/Fox/gLTF/Fox.gltf";
    const std::string assetPath = GetAssetPath(filename);

    Assimp::Importer importer;
    const aiScene* assimpScene = importer.ReadFile(assetPath, 0);
    if (assimpScene)
    {
        LoadSkeleton(*assimpScene, skeleton);
    }

    auto& clip1 = LoadClipFromFile("Models/Fox/gLTF/Fox.gltf", skeleton, 0);
    auto& clip2 = LoadClipFromFile("Models/Fox/gLTF/Fox.gltf", skeleton, 2);

    Model& model = models.emplace_back(filename.c_str());
    model.Load();

    float spacing = 200.0f;

    for (int i = 0; i < skeleton.bones.size(); ++i)
    {
        std::ostringstream id;
        id << "skinMatrix[" << i << "]";
        skinUniformIds.push_back(id.str());
    }

    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            auto entity = entityRegistry.create();

            entityRegistry.emplace<EntityTransform>(entity, vec3(-i * spacing, 0.0f, -j * spacing), quat());
            entityRegistry.emplace<EntityModel>(entity, 0);
            entityRegistry.emplace<Pose>(entity, skeleton.bindPose);

            auto clipNode1 = std::make_unique<ClipNode>(clip1);
            auto clipNode2 = std::make_unique<ClipNode>(clip2);

            auto stateMachineNode = std::make_unique<StateMachineNode>();

            stateMachineNode->AddState(std::move(clipNode1));
            stateMachineNode->AddState(std::move(clipNode2));

            {
                Condition condition{ 1, 1 };
                Transition transition{ 0, 1, 1000.0f, condition };
                stateMachineNode->AddTransition(transition);
            }

            {
                Condition condition{ 1, 0 };
                Transition transition{ 1, 0, 1000.0f, condition };
                stateMachineNode->AddTransition(transition);
            }

            {
                Condition condition{ 1, 2 };
                Transition transition{ 0, 0, 1000.0f, condition };
                stateMachineNode->AddTransition(transition);
            }

            {
                Condition condition{ 1, 2 };
                Transition transition{ 1, 1, 1000.0f, condition };
                stateMachineNode->AddTransition(transition);
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
            transform.position += rootMotion * deltaTime;
        }

        mat4 modelTransform = mat4_cast(transform.orientation);
        modelTransform[3] = vec4(transform.position, 1.0);

        lineRenderer.AddPose(modelTransform, skeleton, skeletonPose, vec4(1));
    }
}

void Demo::Widgets()
{
    ImGui::Begin("Demo");

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::DragFloat("Timescale", &timescale, 0.01, 0.0, 100.0);

    if (ImGui::CollapsingHeader("Render", "RENDER", true, true))
    {
        ImGui::Checkbox("Wireframe", &drawWireframe);
    }

    if (ImGui::CollapsingHeader("Parameters", "PARAMETERS", true, true))
    {
        ImGui::DragInt("P1", &p1, 1.0f, 0, 1);
    }

    scene.AddWidgets();

    ImGui::End();
}
