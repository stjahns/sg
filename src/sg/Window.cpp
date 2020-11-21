#include "Window.h"

#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    }
}

Window::~Window()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

void Window::Update()
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::RenderImGui()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
