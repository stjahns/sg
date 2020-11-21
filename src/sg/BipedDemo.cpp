#include <sstream>
#include <glm/gtc/random.hpp>

#include "LineRenderer.h"
#include "Asset.h"
#include "anim/AssimpSkeletonLoader.h"
#include "anim/AssimpClipLoader.h"

#include <tiny_gltf.h>
#include "BipedDemo.h"
#include "Components.h"

BipedDemo::BipedDemo(Window& window, Camera& camera)
    : shaderProgram("shaders/skinned.vs.glsl", "shaders/mixamo.fs.glsl")
    , camera(camera)
    , drawWireframe(false)
    , drawSkeleton(false) // fixme
    , renderer(window)
{
    LoadBiped();
}

void BipedDemo::LoadSkeletonFromFile(std::string filename, Skeleton& skeleton)
{
    const std::string path = GetAssetPath(filename);
    Assimp::Importer importer;
    const aiScene* assimpScene = importer.ReadFile(path, 0);
    if (assimpScene)
    {
        LoadSkeleton(*assimpScene, skeleton);
    }
}

void BipedDemo::LoadClipFromFile(std::string filename, Skeleton& skeleton, int clipIndex, std::string idPrefix)
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

void BipedDemo::LoadBiped()
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

    LoadClipFromFile("Clips/armada.fbx", skeleton, 0, "mixamorig:");
    LoadClipFromFile("Clips/armada to esquiva.fbx", skeleton, 0, "mixamorig:");
    LoadClipFromFile("Clips/ginga sideways to au.fbx", skeleton, 0, "mixamorig:");
    LoadClipFromFile("Clips/au.fbx", skeleton, 0, "mixamorig:");
    LoadClipFromFile("Clips/au to role.fbx", skeleton, 0, "mixamorig:");

    float spacing = 100.0f;

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            auto entity = entityRegistry.create();

            float heading = glm::linearRand(0.0f, two_pi<float>());
            entityRegistry.emplace<EntityTransform>(entity, vec3(-i * spacing, 0.0f, -j * spacing), quat(vec3(0.0f, heading, 0.0f)));

            entityRegistry.emplace<Pose>(entity, skeleton.bindPose);
            entityRegistry.emplace<EntityModel>(entity, 0);
            entityRegistry.emplace<Skeleton>(entity, skeleton);

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

void BipedDemo::Update(float deltaTime)
{
    auto view = entityRegistry.view<AnimGraph, Pose, EntityTransform>();

    AnimationPose pose;
    
    pose.Resize(skeleton.bones.size()); // TODO skeleton.Size()
 
    for (auto entity : view)
    {
        auto& animGraph = view.get<AnimGraph>(entity);
        animGraph.rootNode->Update(deltaTime, Parameters());

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

void BipedDemo::Render()
{
    glEnable(GL_DEPTH_TEST);

    if (drawWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    auto view = entityRegistry.view<EntityTransform, EntityModel, Pose, Skeleton>();

    for (auto entity : view)
    {
        auto& transform = view.get<EntityTransform>(entity);
        auto& entityModel = view.get<EntityModel>(entity);
        auto& pose = view.get<Pose>(entity);
        auto& skeleton = view.get<Skeleton>(entity);

        Model& model = models[entityModel.modelIndex];

        pose.ComputeObjectFromLocal(skeleton);
    }

    shadowMapRenderer.RenderLightMaps(scene, [&](ShaderProgram& shader)
    {
        for (auto entity : view)
        {
            auto& transform = view.get<EntityTransform>(entity);
            auto& entityModel = view.get<EntityModel>(entity);
            auto& pose = view.get<Pose>(entity);
            auto& skeleton = view.get<Skeleton>(entity);

            shader.SetUniform("model", transform.ToMat4());

            for (int i = 0; i < skeleton.bones.size(); ++i)
            {
                // TODO -- these should be cached on another component?
                mat4 skinMatrix = pose.objectTransforms[i] * skeleton.bones[i].inverseBindPose;
                shader.SetUniform(skinUniformIds[i].c_str(), skinMatrix);
            }

            Model& model = models[entityModel.modelIndex];

            if (model.IsLoaded() && !model.IsBound())
            {
                model.Bind();
            }

            model.Draw();
        }
    });

    renderer.DeferredRender(scene, camera, [&](ShaderProgram& shader)
    {
        for (auto entity : view)
        {
            auto& transform = view.get<EntityTransform>(entity);
            auto& entityModel = view.get<EntityModel>(entity);
            auto& pose = view.get<Pose>(entity);
            auto& skeleton = view.get<Skeleton>(entity);

            mat4 mvp = camera.GetProjection() * camera.GetViewMatrix() * transform.ToMat4();
            shader.SetUniform("model", transform.ToMat4());
            shader.SetUniform("MVP", mvp);

            for (int i = 0; i < skeleton.bones.size(); ++i)
            {
                // TODO -- these should be cached on another component?
                mat4 skinMatrix = pose.objectTransforms[i] * skeleton.bones[i].inverseBindPose;
                shader.SetUniform(skinUniformIds[i].c_str(), skinMatrix);
            }

            Model& model = models[entityModel.modelIndex];

            if (model.IsLoaded() && !model.IsBound())
            {
                model.Bind();
            }

            model.Draw();
        }
    });

    lineRenderer.Render(camera.GetViewMatrix(), camera.GetProjection());

    if (drawWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void BipedDemo::AddWidgets()
{
    if (ImGui::CollapsingHeader("Render"))
    {
        ImGui::Checkbox("Wireframe", &drawWireframe);
        ImGui::Checkbox("Skeleton", &drawSkeleton);
        renderer.AddWidgets();
    }

    scene.AddWidgets();
}
