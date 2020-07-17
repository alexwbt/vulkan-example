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

    const int MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    /*
        If MAX_FRAMES_IN_FLIGHT is higher than the number of swap chain images or vkAcquireNextImageKHR
        returns images out-of-order then it's possible that we may start rendering to a swap chain image
        that is already in flight. To avoid this, we need to track for each swap chain image if a frame in
        flight is currently using it. This mapping will refer to frames in flight by their fences so we'll
        immediately have a synchronization object to wait on before a new frame can use that image.
    */
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

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
        imageAvailableSemaphores = createSemaphores(logicalDevice, MAX_FRAMES_IN_FLIGHT); // semaphore.cpp
        renderFinishedSemaphores = createSemaphores(logicalDevice, MAX_FRAMES_IN_FLIGHT); // semaphore.cpp
        // Create fences.
        inFlightFences = createFences(logicalDevice, MAX_FRAMES_IN_FLIGHT); // fence.cpp
        imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);
    }

    void cleanup()
    {
        // Wait for the logical device to finish operations before clean up.
        vkDeviceWaitIdle(logicalDevice);

        // Destroy fences.
        destroyFences(logicalDevice, inFlightFences); // fence.cpp
        // Destroy semaphores.
        destroySemaphores(logicalDevice, renderFinishedSemaphores); // semaphore.cpp
        destroySemaphores(logicalDevice, imageAvailableSemaphores); // semaphore.cpp

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
        vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

        // Acquiring an image from the swap chain.
        uint32_t imageIndex;
        vkAcquireNextImageKHR(logicalDevice, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        // Reset fence before using it.
        vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

        // Submitting the command buffer.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit command buffer.");
        }

        // Presentation.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(presentQueue, &presentInfo);
        
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
