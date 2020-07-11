#define DEBUG 1
// Local Headers
#include "sg/glitter.hpp"
#include "sg/ShaderProgram.h"
#include "sg/defines.h"

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // value_ptr

#include <imgui.h>

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
#include <tiny_gltf.h>

#include "sg/Model.h"
#include "sg/Camera.h"
#include "sg/Light.h"
#include "sg/Scene.h"
#include "sg/Renderer.h"
#include "sg/LineRenderer.h"
#include "sg/GlfwWindow.h"

using namespace glm;

int main(int /*argc*/, char** /*argv*/) 
{
	GLFWWindow window(1280, 800);

	if (!window.Valid())
	{
		fprintf(stderr, "Failed to create OpenGL context");
		return EXIT_FAILURE;
	}

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

	Scene scene;

	Renderer renderer;

	glEnable(GL_MULTISAMPLE);

	LineRenderer lineRenderer;

	double time = glfwGetTime();

    window.AddMouseEventHandler([&](MouseEvent e) { 
        scene.GetCamera().OnMouseEvent(e);
    });

	while (!window.ShouldClose())
	{
		window.Update();

		if (window.GetKey(GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			window.Close();
		}

        if (window.GetKey(GLFW_KEY_R) == GLFW_PRESS)
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

		double newTime = glfwGetTime();
		double deltaTime = newTime - time;
		time = newTime;

		scene.Update(lineRenderer, activeShader, deltaTime * 1000.0f);

		scene.AddWidgets();
		renderer.Widgets();

		ImGui::End();

        // Background Fill Color
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Camera& camera = scene.GetCamera();
		camera.Update(window);
		mat4 view = camera.GetViewMatrix();

		glEnable(GL_DEPTH_TEST);

		glUseProgram(activeShader.GetHandle());

		// build a model-view-projection

		WindowSize windowSize = window.GetWindowSize();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)windowSize.width / (float)windowSize.height, 0.01f, 10000.0f);
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

		window.SwapBuffers();
    }

    return EXIT_SUCCESS;
}
