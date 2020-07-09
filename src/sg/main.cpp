#define DEBUG 1
// Local Headers
#include "glitter.hpp"
#include "ShaderProgram.h"
#include "defines.h"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // value_ptr

#include <imgui.h>
#include "imgui_impl_glfw_glad.h"

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>

// TODO kill this?
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "Model.h"
#include "Camera.h"
#include "Light.h"
#include "Scene.h"
#include "Renderer.h"
#include "LineRenderer.h"

using namespace glm;

Scene* pScene = nullptr;

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (pScene)
	{
		pScene->GetCamera().OnMouseEvent(window, button, action, mods);
	}
}

// TODO -- don't use 'mstreamingwindow'
// TODO -- shader manager
// TODO -- dropdown for different demos?
// TODO -- resourse cleanup

int main(int /*argc*/, char** /*argv*/) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow *mWindow = glfwCreateWindow(mWidth, mHeight, "SGDemo", nullptr, nullptr);

	// Check for Valid Context
    if (mWindow == nullptr) 
	{
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    // Setup ImGui binding
    ImGui_ImplGlfwGlad_Init(mWindow, true);

	int selectedShaderIndex = 0;

	const char *shaderProgramItems[] = {
		"Unlit Skinned"
		"Unlit",
		"Lit, Untextured",
		"Lit, Textured"
	};

	ShaderProgram shaderPrograms[] = 
	{
		ShaderProgram("shaders/skinned.vs.glsl", "shaders/unlit.fs.glsl"),
		ShaderProgram("shaders/mvp.vs.glsl", "shaders/unlit.fs.glsl"),
		ShaderProgram("shaders/mvp.vs.glsl", "shaders/light0.fs.glsl"),
		//ShaderProgram("shaders/mvp.vs.glsl", "shaders/default.fs.glsl"),
		ShaderProgram("shaders/mvp.vs.glsl", "shaders/default.fs.glsl"),
	};

	// TODO -- if shader compiles fail, avoid crashing!

	ShaderProgram& activeShader = shaderPrograms[selectedShaderIndex];


	glUseProgram(activeShader.GetHandle());

	bool bWireframe = false;

	glfwSetMouseButtonCallback(mWindow, mouseCallback);

	Scene scene;
	pScene = &scene;

	Renderer renderer;

	glEnable(GL_MULTISAMPLE);

	LineRenderer lineRenderer;

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {

        glfwPollEvents();
        ImGui_ImplGlfwGlad_NewFrame();

        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

		if (glfwGetKey(mWindow, GLFW_KEY_R) == GLFW_PRESS)
		{
			for (int i = 0; i < sizeof(shaderPrograms) / sizeof(shaderPrograms[0]); ++i)
			{
				shaderPrograms[i].Reload();
			}
		}

		ImVec4 clear_color = ImColor(0.2f, 0.2f, 0.2f, 1.0f);

		ImGui::Begin("Scene");

		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Combo("Shader Program", &selectedShaderIndex, shaderProgramItems, sizeof(shaderProgramItems) / sizeof(shaderProgramItems[0]));

		if (ImGui::Button("Reload Shaders"))
		{
			for (int i = 0; i < sizeof(shaderPrograms) / sizeof(shaderPrograms[0]); ++i)
			{
				shaderPrograms[i].Reload();
			}
		}

		ImGui::Checkbox("Wireframe", &bWireframe);

		ShaderProgram& activeShader = shaderPrograms[selectedShaderIndex];

		scene.Update(lineRenderer, activeShader);

		scene.AddWidgets();
		renderer.Widgets();

		ImGui::End();


        // Background Fill Color
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Camera& camera = scene.GetCamera();
		camera.Update(mWindow);
		mat4 view = camera.GetViewMatrix();

		glEnable(GL_DEPTH_TEST);

		glUseProgram(activeShader.GetHandle());

		// build a model-view-projection
		GLint w, h;
		glfwGetWindowSize(mWindow, &w, &h);

		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.01f, 10000.0f);
		camera.SetProjection(projection);

		if (bWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		renderer.RenderDirectionalLightShadowMap(scene);
		renderer.RenderPointLightShadowMaps(scene);

		renderer.ForwardRender(scene, activeShader);
		//renderer.DeferredRender(scene);

		if (bWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		scene.DrawLights();

        lineRenderer.AddLine(vec3(0), vec3(1, 0, 0), vec4(1, 0, 0, 1), vec4(1, 0, 0, 0));
        lineRenderer.AddLine(vec3(0), vec3(0, 1, 0), vec4(0, 1, 0, 1), vec4(0, 1, 0, 0));
        lineRenderer.AddLine(vec3(0), vec3(0, 0, 1), vec4(0, 0, 1, 1), vec4(0, 0, 1, 0));
		lineRenderer.Render(view, projection);

        ImGui::Render();

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
    }

    // Cleanup
    ImGui_ImplGlfwGlad_Shutdown();
    glfwTerminate();

    return EXIT_SUCCESS;
}
