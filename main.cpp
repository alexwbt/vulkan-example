#include <iostream>
#include <stdexcept>

#include "window.h"
#include "vulkan.h"
#include "device.h"

class HelloTriangleApplication
{
public:
	void run()
	{
		init();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice device;

	void init()
	{
		// Initialize GLFW and create window.
		// window_initialization.cpp
		window = initWindow();

		// Application configuration for creating Vulkan instance.
		// (technically optional)
		VkApplicationInfo appInfo = createHelloTriangleAppInfo();
		// Create Vuklan instance, list available extensions and apply validation layers(debug mode only).
		// vulkan_initialization.cpp
		instance = initVulkan(appInfo);

		// Select best suited physical device(graphics card).
		device = selectPhysicalDevice(instance);
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}

	void cleanup()
	{
		// Destroy window and terminate GLFW.
		// window_initialization.cpp
		terminateWindow(window);

		// Destroy Vulkan instance
		// vulkan_initialization.cpp
		destroyVulkanInstance(instance);
	}
};

int main()
{
	HelloTriangleApplication app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
