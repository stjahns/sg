#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <functional>

struct WindowSize
{
    int width;
    int height;
};

struct MouseEvent
{
    double mouseX;
    double mouseY;
    int button;
    int action;
    int mods;
};

typedef std::function<void(MouseEvent)> MouseEventHandler;
typedef std::function<void(int, int)> ResizeEventHandler;

class Window
{
public:

    Window(int width, int height);

    ~Window();

    bool Valid() { return window != nullptr; }
    bool ShouldClose() { return glfwWindowShouldClose(window); }

    void Update();

    void SwapBuffers()
    {
        glfwSwapBuffers(window);
    }

    WindowSize GetWindowSize()
    {
        GLint w, h;
        glfwGetWindowSize(window, &w, &h);
        return { w, h };
    }

    void GetCursorPos(double& mouseX, double& mouseY)
    {
        glfwGetCursorPos(window, &mouseX, &mouseY);
    }

    int GetKey(int key)
    {
        return glfwGetKey(window, key);
    }

    void Close()
    {
        glfwSetWindowShouldClose(window, true);
    }

    void AddMouseEventHandler(MouseEventHandler handler)
    {
        mouseEventHandlers.push_back(handler);
    }

    void AddResizeEventHandler(ResizeEventHandler handler)
    {
        resizeEventHandlers.push_back(handler);
    }

private:

    void OnMouseButtonAction(int button, int action, int mods);
    void OnFramebufferResized(int width, int height);

    std::vector<MouseEventHandler> mouseEventHandlers;
    std::vector<ResizeEventHandler> resizeEventHandlers;

    GLFWwindow* window;

    static Window* instance;

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
};