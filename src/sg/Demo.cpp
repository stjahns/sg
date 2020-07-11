#include <sstream>

#include "LineRenderer.h"
#include "Asset.h"
#include "anim/AssimpSkeletonLoader.h"
#include "anim/AssimpClipLoader.h"

#include <tiny_gltf.h>

#include "Demo.h"

Demo::Demo(GLFWWindow& window) 
    : window(window)
    , shaderProgram("shaders/skinned.vs.glsl", "shaders/unlit.fs.glsl")
    , drawWireframe(false)
    , p1(0.0f)
{
    window.AddMouseEventHandler([&](MouseEvent e) { scene.GetCamera().OnMouseEvent(e); });

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

    Assimp::Importer importer;
    const aiScene* assimpScene = importer.ReadFile(assetPath, 0);
    if (assimpScene)
    {
        LoadClip(*assimpScene, skeleton, clip1, 0);
        LoadClip(*assimpScene, skeleton, clip2, 2);

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

    scene.LoadModel(filename.c_str());

    time = glfwGetTime();
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

        Update(deltaTime * 1000.0f); // can we pick ms vs s?

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

// TODO - safer type for time (ms, s??)
void Demo::Update(float deltaTime)
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

    shaderProgram.Use();

    for (int i = 0; i < skeleton.bones.size(); ++i)
    {
        mat4 skinMatrix = skeleton.currentPose.objectTransforms[i] * skeleton.bones[i].inverseBindPose;

        std::ostringstream id;
        id << "skinMatrix[" << i << "]";
        shaderProgram.SetUniform((id.str()).c_str(), skinMatrix);
    }

    lineRenderer.AddPose(skeleton, skeleton.currentPose, vec4(1));
}

void Demo::Widgets()
{
    ImGui::Begin("Demo");

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if (ImGui::CollapsingHeader("Render", "RENDER", true, true))
    {
        ImGui::Checkbox("Wireframe", &drawWireframe);
    }

    if (ImGui::CollapsingHeader("Parameters", "PARAMETERS", true, true))
    {
        ImGui::DragInt("P1", &p1, 1.0f, 0, 1);
    }

    ImGui::End();
}
