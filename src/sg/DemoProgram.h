#pragma once

#include "Camera.h"
#include "GlfwWindow.h"
#include "BipedDemo.h"
#include "SceneDemo.h"

class DemoProgram
{
    public:

        DemoProgram(GLFWWindow& window);

        void Run();

    private:

        void Update();
        void Render();
        void AddWidgets();

        GLFWWindow& window;

        Camera camera;
        //BipedDemo demo;
        SceneDemo demo;

        double elapsedTime;
        float timescale;
};