#include <iostream>
#include <stdexcept>

#include "window.h"
#include "vulkan.h"
#include "physical_device.h"
#include "logical_device.h"

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
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;

    void init()
    {
        // Initialize GLFW and create window.
        window = initWindow(); // window.cpp

        // Application configuration for creating Vulkan instance.
        // (technically optional)
        VkApplicationInfo appInfo = createHelloTriangleAppInfo(); // vulkan.cpp
        // Create Vuklan instance, list available extensions and apply validation layers(debug mode only).
        instance = initVulkan(appInfo); // vulkan.cpp

        // Select best suited physical device(graphics card).
        physicalDevice = selectPhysicalDevice(instance); // physical_device.cpp

        // Create logical device to interface physical device.
        logicalDevice = createLogicalDevice(physicalDevice, &graphicsQueue); // logical_device.cpp
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
        terminateWindow(window); // window.cpp

        // Destroy Vulkan instance.
        destroyVulkanInstance(instance); // vulkan.cpp

        // Destroy logical device.
        destroyLogicalDevice(logicalDevice); // logical_device.cpp
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
