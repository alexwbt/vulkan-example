#include "vulkan_example.h"

#include <stdexcept>

namespace VulkanExample
{

    VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window)
    {
        VkSurfaceKHR surface;

        // Create window surface.
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface.");
        }

        return surface;
    }

    void destroySurface(VkInstance instance, VkSurfaceKHR surface)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

}
