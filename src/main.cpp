#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "sg/GlfwWindow.h"
#include "sg/Demo.h"

using namespace glm;

int main(int /*argc*/, char** /*argv*/) 
{
	GLFWWindow window(1280, 800);

	if (!window.Valid())
	{
		fprintf(stderr, "Failed to create OpenGL context");
		return EXIT_FAILURE;
	}

	Demo demo(window);

	demo.Run();

    return EXIT_SUCCESS;
}
