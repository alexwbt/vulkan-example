#include <iostream>
#include <stdexcept>

#include "window_initialization.h"
#include "vulkan_initialization.h"

class HelloTriangleApplication {
public:
    void run() {
        init();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VkInstance instance;

    void init() {
        // Initialize GLFW and create window.
        // window_initialization.cpp
        window = initWindow();

        // Application configuration for creating Vulkan instance.
        // (technically optional)
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        // Create Vulkan instance and ...
        // vulkan_initialization.cpp
        instance = initVulkan(appInfo);
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        // Destroy window and terminate GLFW.
        // window_initialization.cpp
        terminateWindow(window);

        // Destroy Vulkan instance
        // vulkan_initialization.cpp
        destroyVulkanInstance(instance);
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
