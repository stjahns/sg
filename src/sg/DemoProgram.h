#pragma once

#include "Camera.h"
#include "Window.h"
#include "BipedDemo.h"
#include "SceneDemo.h"

class DemoProgram
{
    public:

        DemoProgram(Window& window);

        void Run();

    private:

        void Update();
        void Render();
        void AddWidgets();

        Window& window;

        Camera camera;
        BipedDemo demo;
        //SceneDemo demo;

        double elapsedTime;
        float timescale;
};