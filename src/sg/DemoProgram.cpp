#include "DemoProgram.h"

DemoProgram::DemoProgram(Window& window) 
    : window(window)
    , demo(camera)
    , elapsedTime(glfwGetTime())
    , timescale(1.0f)
{
    window.AddMouseEventHandler([&](MouseEvent e) { camera.OnMouseEvent(e); });
}

void DemoProgram::Run()
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
            demo.Reload();
        }

        Update();
        Render();

        AddWidgets();
        ImGui::Render();

        window.SwapBuffers();
    }
}

void DemoProgram::Update()
{
    double deltaTime = glfwGetTime() - elapsedTime;
    elapsedTime += deltaTime;

    camera.Update(window);

    demo.Update(deltaTime * timescale);
}

void DemoProgram::Render()
{
    const vec4 clearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    WindowSize windowSize = window.GetWindowSize();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)windowSize.width / (float)windowSize.height, 0.01f, 10000.0f);
    camera.SetProjection(projection);

    demo.Render();
}

void DemoProgram::AddWidgets()
{
    ImGui::Begin("Demo");

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::DragFloat("Timescale", &timescale, 0.01, 0.0, 1000.0);

    camera.AddWidgets();
    demo.AddWidgets();

    ImGui::End();
}
