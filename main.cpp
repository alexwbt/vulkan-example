#include <iostream>

#include "vulkan_example.h"

using namespace VulkanExample;

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

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkFramebuffer> swapchainFramebuffers;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

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

        // Create render pass.
        renderPass = createRenderPass(logicalDevice, swapchainImageFormat); // graphics.cpp
        // Create pipline layout.
        pipelineLayout = createPipelineLayout(logicalDevice); // graphics.cpp
        // Create graphics pipeline
        graphicsPipeline = createGraphicsPipeline(logicalDevice, swapchainExtent, renderPass, pipelineLayout); // graphics.cpp

        // Create framebuffers.
        swapchainFramebuffers = createFramebuffers(logicalDevice, swapchainImageViews, renderPass, swapchainExtent); // framebuffer.cpp

        // Create command pool.
        commandPool = createCommandPool(logicalDevice, physicalDevice, surface); // command.cpp
        // Allocate command buffers.
        commandBuffers = allocateCommandBuffers(logicalDevice, commandPool, swapchainFramebuffers.size()); // command.cpp
        // Begin render pass.
        beginRenderPass(renderPass, swapchainExtent, graphicsPipeline, commandBuffers, swapchainFramebuffers); // command.cpp

        // Create semaphores.
        imageAvailableSemaphore = createSemaphore(logicalDevice); // semaphore.cpp
        renderFinishedSemaphore = createSemaphore(logicalDevice); // semaphore.cpp
    }

    void cleanup()
    {
        // Destroy semaphores.
        destroySemaphore(logicalDevice, renderFinishedSemaphore); // semaphore.cpp
        destroySemaphore(logicalDevice, imageAvailableSemaphore); // semaphore.cpp

        // Destroy command pool.
        destroyCommandPool(logicalDevice, commandPool); // command.cpp

        // Destroy framebuffers.
        destroyFramebuffers(logicalDevice, swapchainFramebuffers); // framebuffer.cpp

        // Destroy graphics pipeline.
        destroyGraphicsPipeline(logicalDevice, graphicsPipeline); // graphics.cpp
        // Destroy pipline layout.
        destroyPipelineLayout(logicalDevice, pipelineLayout); // graphics.cpp
        // Destroy render pass.
        destroyRenderPass(logicalDevice, renderPass); // graphics.cpp

        // Destroy image views.
        destroyImageViews(logicalDevice, swapchainImageViews); // image.cpp

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

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            render();
        }
    }

    /*
        The render function will perform the following operations:

        - Acquire an image from the swap chain
        - Execute the command buffer with that image as attachment in the framebuffer
        - Return the image to the swap chain for presentation

        Each of these events is set in motion using a single function call,
        but they are executed asynchronously. The function calls will return before
        the operations are actually finished and the order of execution is also undefined.
        That is unfortunate, because each of the operations depends on the previous one finishing.

        There are two ways of synchronizing swap chain events: fences and semaphores.
        They're both objects that can be used for coordinating operations by having one
        operation signal and another operation wait for a fence or semaphore to go from
        the unsignaled to signaled state.

        The difference is that the state of fences can be accessed from your program using
        calls like vkWaitForFences and semaphores cannot be. Fences are mainly designed to
        synchronize your application itself with rendering operation, whereas semaphores are
        used to synchronize operations within or across command queues. We want to synchronize
        the queue operations of draw commands and presentation, which makes semaphores the best fit.
    */
    void render()
    {
        // Acquiring an image from the swap chain.
        uint32_t imageIndex;
        vkAcquireNextImageKHR(logicalDevice, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        // Submitting the command buffer.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit command buffer.");
        }
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
