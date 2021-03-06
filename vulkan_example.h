#pragma once

#pragma warning (disable : 26812)

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>
#include <optional>
#include <array>

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        // One attribute description per attribute
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

class VK
{
public:
    static GLFWwindow* window;
    static int width, height;

    static VkInstance instance;

    static VkSurfaceKHR surface;

    static VkPhysicalDevice physicalDevice;

    static VkDevice logicalDevice;
    static VkQueue graphicsQueue;
    static VkQueue presentQueue;

    static VkSwapchainKHR swapchain;
    static std::vector<VkImage> swapchainImages;
    static std::vector<VkImageView> swapchainImageViews;
    static VkFormat swapchainImageFormat;
    static VkExtent2D swapchainExtent;

    static VkRenderPass renderPass;
    static VkPipelineLayout pipelineLayout;
    static VkPipeline graphicsPipeline;

    static std::vector<VkFramebuffer> swapchainFramebuffers;

    static VkCommandPool commandPool;
    static std::vector<VkCommandBuffer> commandBuffers;

    static const int MAX_FRAMES_IN_FLIGHT = 2;
    static std::vector<VkSemaphore> imageAvailableSemaphores;
    static std::vector<VkSemaphore> renderFinishedSemaphores;
    static std::vector<VkFence> inFlightFences;
    static std::vector<VkFence> imagesInFlight;
    static size_t currentFrame;

    static bool framebufferResized;

    static void initWindow();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void terminateWindow();
    static void initVulkan(VkApplicationInfo info);
    static void destroyVulkanInstance();
    static void createSurface();
    static void destroySurface();
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    static void selectPhysicalDevice();
    static void waitIdle();
    static void createLogicalDevice();
    static void destroyLogicalDevice();
    static bool checkValidationLayerSupport();
    static void createSwapchain();
    static void destroySwapchain();
    static void retrieveSwapchainImages();
    static void createImageViews();
    static void destroyImageViews();
    static void createRenderPass();
    static void destroyRenderPass();
    static void createPipelineLayout();
    static void destroyPipelineLayout();
    static void createGraphicsPipeline();
    static void destroyGraphicsPipeline();
    static void createFramebuffers();
    static void freeCommandBuffers();
    static void destroyFramebuffers();
    static void createCommandPool();
    static void destroyCommandPool();
    static void allocateCommandBuffers();
    static void beginRenderPass();
    static void createSyncObjects();
    static void destroySyncObjects();

    static void createVertexBuffer();
    static void destroyVertexBuffer();

    static void init(VkApplicationInfo appInfo);
    static void cleanup();
    static void initSwapchain();
    static void cleanupSwapchain();
    static void recreateSwapchain();
    static void render();

};
