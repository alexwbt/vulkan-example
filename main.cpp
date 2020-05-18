#include <iostream>
#include <stdexcept>

#include "window.h"
#include "vulkan.h"
#include "physical_device.h"
#include "logical_device.h"
#include "surface.h"
#include "swapchain.h"
#include "image.h"

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

    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;

    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;

    void init()
    {
        // Initialize GLFW and create window.
        window = initWindow(); // window.cpp

        // AppInfo: Application configuration for creating Vulkan instance. (technically optional)
        // Create Vuklan instance, list available extensions and apply validation layers(debug mode only).
        instance = initVulkan(createHelloTriangleAppInfo()); // vulkan.cpp

        // Create window surface to establish the connection between Vulkan and the window system to present results to the screen.
        // Needs to be created right after the instance creation.
        surface = createSurface(instance, window); // surface.cpp

        // Select best suited physical device(graphics card).
        physicalDevice = selectPhysicalDevice(instance, surface); // physical_device.cpp

        // Create logical device and retrieve queue handle to interface physical device.
        logicalDevice = createLogicalDevice(physicalDevice, surface, &graphicsQueue, &presentQueue); // logical_device.cpp

        // Create swap chain(frame buffer) and retrieve swap chain image format and extent.
        swapchain = createSwapchain(physicalDevice, logicalDevice, surface, &swapchainImageFormat, &swapchainExtent); // swapchain.cpp
        // Revrieve handles of VkImages from swap chain.
        swapchainImages = retrieveSwapchainImages(logicalDevice, swapchain); // image.cpp
        // Create image views.
        swapchainImageViews = createImageViews(swapchainImages, swapchainImageFormat, logicalDevice); // image.cpp
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
        // Destroy image views.
        destroyImageViews(logicalDevice, swapchainImageViews);

        // Destroy swap chain.
        destroySwapchain(logicalDevice, swapchain); // swapchain.cpp

        // Destroy logical device (before Vulkan instance).
        destroyLogicalDevice(logicalDevice); // logical_device.cpp

        // Destroy window surface.
        destroySurface(instance, surface); // surface.cpp

        // Destroy Vulkan instance.
        destroyVulkanInstance(instance); // vulkan.cpp

        // Destroy window and terminate GLFW.
        terminateWindow(window); // window.cpp
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
