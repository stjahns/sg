#include "Window.h"

#include <imgui.h>
#include "imgui_impl_glfw_glad.h"

Window::Window(int width, int height)
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
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    }
}

Window::~Window()
{
    ImGui_ImplGlfwGlad_Shutdown();
    glfwTerminate();
}

void Window::Update()
{
    glfwPollEvents();
    ImGui_ImplGlfwGlad_NewFrame();
}

void Window::OnMouseButtonAction(int button, int action, int mods)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    MouseEvent mouseEvent{ mouseX, mouseY, button, action, mods };

    for (auto& handler : mouseEventHandlers)
    {
        handler(mouseEvent);
    }
}

void Window::OnFramebufferResized(int width, int height)
{
    for (auto& handler : resizeEventHandlers)
    {
        handler(width, height);
    }
}

Window* Window::instance = nullptr;

void Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (instance)
    {
        instance->OnMouseButtonAction(button, action, mods);
    }
}

void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    if (instance)
    {
        instance->OnFramebufferResized(width, height);
    }
}
