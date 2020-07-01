#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <optional>
#include <stdexcept>

namespace VulkanExample
{
    /*
        window.cpp
    */

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    GLFWwindow* initWindow(); // window.cpp
    void terminateWindow(GLFWwindow* window); // window.cpp

    /*
        vulkan.cpp
    */

    VkInstance initVulkan(VkApplicationInfo info);
    void destroyVulkanInstance(VkInstance instance);

    VkApplicationInfo createHelloTriangleAppInfo();

    /*
        surface.cpp
    */

    VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window);
    void destroySurface(VkInstance instance, VkSurfaceKHR surface);

    /*
        physical_device.cpp
    */

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

    /*
        logical_device.cpp
    */

    VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue* graphicsQueue, VkQueue* presentQueue);
    void destroyLogicalDevice(VkDevice device);

    /*
        queue.cpp
    */

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily; // For rendering (drawing commands).
        std::optional<uint32_t> presentFamily; // For presetning render to window surface.

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    /*
        layer.cpp
    */

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    bool checkValidationLayerSupport();

    /*
        swapchain.cpp
    */

    VkSwapchainKHR createSwapchain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, VkFormat* swapchainImageFormat, VkExtent2D* swapchainExtent);
    void destroySwapchain(VkDevice logicalDevice, VkSwapchainKHR swapchain);

    /*
        image.cpp
    */

    std::vector<VkImage> retrieveSwapchainImages(VkDevice logicalDevice, VkSwapchainKHR swapchain);

    std::vector<VkImageView> createImageViews(std::vector<VkImage> images, VkFormat imageFormat, VkDevice logicalDevice);
    void destroyImageViews(VkDevice logicalDevice, std::vector<VkImageView> imageViews);

    /*
        graphics.cpp
    */

    VkRenderPass createRenderPass(VkDevice logicalDevice, VkFormat swapchainImageFormat);
    void destroyRenderPass(VkDevice logicalDevice, VkRenderPass renderPass);

    VkPipelineLayout createPipelineLayout(VkDevice logicalDevice);
    void destroyPipelineLayout(VkDevice logicalDevice, VkPipelineLayout pipelineLayout);

    VkPipeline createGraphicsPipeline(VkDevice logicalDevice, VkExtent2D swapchainExtent, VkRenderPass renderPass, VkPipelineLayout pipelineLayout);
    void destroyGraphicsPipeline(VkDevice logicalDevice, VkPipeline graphicsPipeline);

    /*
        framebuffer.cpp
    */

    std::vector<VkFramebuffer> createFramebuffers(VkDevice logicalDevice, std::vector<VkImageView> swapchainImageViews, VkRenderPass renderPass, VkExtent2D swapchainExtent);
    void destroyFramebuffers(VkDevice logicalDevice, std::vector<VkFramebuffer> swapchainFramebuffers);

    /*
        command.cpp
    */

    VkCommandPool createCommandPool(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    void destroyCommandPool(VkDevice logicalDevice, VkCommandPool commandPool);

    std::vector<VkCommandBuffer> allocateCommandBuffers(VkDevice logicalDevice, VkCommandPool commandPool, size_t size);
    void beginRenderPass(VkRenderPass renderPass, VkExtent2D swapchainExtent, VkPipeline graphicsPipeline, std::vector<VkCommandBuffer> commandBuffers, std::vector<VkFramebuffer> swapchainFramebuffers);
}
