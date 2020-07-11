#include "GlfwWindow.h"

#include <imgui.h>
#include "imgui_impl_glfw_glad.h"

GLFWWindow::GLFWWindow(int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(width, height, "SGDemo", nullptr, nullptr);

    if (window)
    {
        glfwMakeContextCurrent(window);
        gladLoadGL();
        fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

        instance = this;

        ImGui_ImplGlfwGlad_Init(window, true);

        glfwSetMouseButtonCallback(window, MouseButtonCallback);
    }
}

GLFWWindow::~GLFWWindow()
{
    ImGui_ImplGlfwGlad_Shutdown();
    glfwTerminate();
}

void GLFWWindow::Update()
{
    glfwPollEvents();
    ImGui_ImplGlfwGlad_NewFrame();
}

void GLFWWindow::OnMouseButtonAction(GLFWwindow* window, int button, int action, int mods)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    MouseEvent mouseEvent{ mouseX, mouseY, button, action, mods };

    for (auto& handler : mouseEventHandlers)
    {
        handler(mouseEvent);
    }
}

GLFWWindow* GLFWWindow::instance = nullptr;

void GLFWWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (instance)
    {
        instance->OnMouseButtonAction(window, button, action, mods);
    }
}
