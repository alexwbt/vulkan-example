#include "vulkan_example.h"

#include <iostream>
#include <cstdint>
#include <set>
#include <algorithm>
#include <stdexcept>

#include "util.h"
#include "vertex_buffer.h"

GLFWwindow* VulkanExample::window;
VkInstance VulkanExample::instance;
VkSurfaceKHR VulkanExample::surface;
VkPhysicalDevice VulkanExample::physicalDevice;
VkDevice VulkanExample::logicalDevice;
VkQueue VulkanExample::graphicsQueue;
VkQueue VulkanExample::presentQueue;
VkSwapchainKHR VulkanExample::swapchain;
std::vector<VkImage> VulkanExample::swapchainImages;
std::vector<VkImageView> VulkanExample::swapchainImageViews;
VkFormat VulkanExample::swapchainImageFormat;
VkExtent2D VulkanExample::swapchainExtent;
VkRenderPass VulkanExample::renderPass;
VkPipelineLayout VulkanExample::pipelineLayout;
VkPipeline VulkanExample::graphicsPipeline;
std::vector<VkFramebuffer> VulkanExample::swapchainFramebuffers;
VkCommandPool VulkanExample::commandPool;
std::vector<VkCommandBuffer> VulkanExample::commandBuffers;
std::vector<VkSemaphore> VulkanExample::imageAvailableSemaphores;
std::vector<VkSemaphore> VulkanExample::renderFinishedSemaphores;
std::vector<VkFence> VulkanExample::inFlightFences;
std::vector<VkFence> VulkanExample::imagesInFlight;
size_t VulkanExample::currentFrame = 0;
bool VulkanExample::framebufferResized = false;

void VulkanExample::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tell GLFW to not create an OpenGL context.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Disable resizing window.

    window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}
void VulkanExample::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    framebufferResized = true;
}
void VulkanExample::terminateWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
void VulkanExample::initVulkan(VkApplicationInfo info)
{
    /*
        CreateInfo tells the Vulkan driver which global extensions
        and validation layers to use. (not optional)
    */
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &info;

    // Enable global extensions.
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // GLFW built-in function: returns required extensions.
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // Enable global validation layers.
    createInfo.enabledLayerCount = 0;

    /*
        Check extension support.
    */
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr); // Get number of available extensions.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()); // Get properties of available extensions.
    // (optional first parameter to filter extensions by validation layer)

    // Listing supported extensions
    std::cout << "available extensions:\n";
    for (const auto& extension : extensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }

    /*
        Check validation layer support. (only in debug mode)
    */
    if (enableValidationLayers)
    {
        if (!checkValidationLayerSupport()) // layer.cpp
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    /*
        Create Vulkan instance.
    */
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance.");
    }
}
void VulkanExample::destroyVulkanInstance()
{
    vkDestroyInstance(instance, nullptr);
}
void VulkanExample::createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.");
    }
}
void VulkanExample::destroySurface()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
}
QueueFamilyIndices VulkanExample::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // Get number of available queue families.

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); // Get properties of available queue families.

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        // Find queue family that supports VK_QUEUE_GRAPHICS_BIT (rendering)
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        // Find queue familt that has presentation support. (presents rendered image to window)
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }
    }

    return indices;
}
bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
    // Get number of supported extensions from device.
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    // Get properties of supported extensions from device.
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    // Clone list of required extensions
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    // Check if all required extensions are listed in availableExtensions.
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}
bool isDeviceSuitable(VkPhysicalDevice physicalDevice)
{
    QueueFamilyIndices indices = VulkanExample::findQueueFamilies(physicalDevice); // queue.cpp

    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

    return indices.isComplete() && extensionsSupported;
}
void VulkanExample::selectPhysicalDevice()
{
    physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); // Get number of available devices.
    if (deviceCount == 0)
    {
        // Throw error if there are no available device.
        throw std::runtime_error("Failed to find GPU with Vulkan support.");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()); // Get all available devices.

    // Select first suitable device.
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }
    if (physicalDevice == VK_NULL_HANDLE) {
        // Throw error if there are no suitable device.
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}
void VulkanExample::waitIdle()
{
    vkDeviceWaitIdle(logicalDevice);
}
void VulkanExample::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    // Create multiple queue families that are necessary for the required queue.
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // List of create info struct.
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() }; // required queue.
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        // Specify number of required queues(to be created) for a single family
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        // Assign priorities to influence the scheduling of command buffer execution.
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Specify required device features. (e.g. geometry shaders)
    // Empty for now
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Create logical device.
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    /*
        The remainder of the information bears a resemblance to
        the VkInstanceCreateInfo struct and requires you to specify
        extensions and validation layers. The difference is that
        these are device specific this time.

        An example of a device specific extension is VK_KHR_swapchain,
        which allows you to present rendered images from that device to windows.
        It is possible that there are Vulkan devices in the system that
        lack this ability, for example because they only support compute operations.

        Previous implementations of Vulkan made a distinction between
        instance and device specific validation layers, but this is no longer the case.
        That means that the enabledLayerCount and ppEnabledLayerNames fields
        of VkDeviceCreateInfo are ignored by up-to-date implementations.
        However, it is still a good idea to set them anyway to be
        compatible with older implementations:
    */

    // Enable requied extensions.
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()); // physical_device.h
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Enable validation layers. (in debug mode)
    if (enableValidationLayers) // layer.h
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()); // layer.h
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    // Create logical device.
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device.");
    }

    // Retrieve queue handles for each queue family.
    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}
void VulkanExample::destroyLogicalDevice()
{
    /*
        Logical devices don't interact directly with instances,
        which is why it's not included as a parameter.
    */
    vkDestroyDevice(logicalDevice, nullptr);
}
bool VulkanExample::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // Get number of available layers.
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());  // Get properties of available layers.

    // Check if all of the layers in validationLayers exist in the availableLayers list.
    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}
SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device)
{
    SwapchainSupportDetails details;

    // Get basic surface capabilities.
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VulkanExample::surface, &details.capabilities);

    // Get supported surface formats.
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanExample::surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, VulkanExample::surface, &formatCount, details.formats.data());
    }

    // Get supported present modes.
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanExample::surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, VulkanExample::surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    // Each VkSurfaceFormatKHR entry contains a format and a colorSpace member.
    for (const auto& availableFormat : availableFormats)
    {
        // Check if SRGB is available.
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    // Use first format if failed.
    return availableFormats[0];
}
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        // Check if VK_PRESENT_MODE_MAILBOX_KHR is available.
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    // Only the VK_PRESENT_MODE_FIFO_KHR mode is guanranteed to be available.
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(VulkanExample::window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // Clamp WIDTH and HEIGHT to be supported by the implementation.
        actualExtent.width = std::max(capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
void VulkanExample::createSwapchain()
{
    // Get supported format, present mode and extent.
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);

    // Choose best of these three settings.
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

    // Specify how many images would have in the swap chain.
    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    // Check if imageCount exceed supported maximum image count. (0 means no maximum)
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    // Swap chain create info.
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // Always 1 unless developing a stereoscopic 3D application.
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    /*
        Specify how to handle swap chain images that will be used across multiple queue families.
        That will be the case in our application if the graphics queue family is different from the presentation queue.
        We'll be drawing on the images in the swap chain from the graphics queue and then submitting them on the presentation queue.

        There are two ways to handle images that are accessed from multiple queues:
        - VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explicitly transfered
                                     before using it in another queue family. This option offers the best performance.
        - VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership transfers.
    */
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    /*
        We can specify that a certain transform should be applied to images in the swap chain
        if it is supported (supportedTransforms in capabilities), like a 90 degree clockwise rotation or horizontal flip.
        To specify that you do not want any transformation, simply specify the current transformation.
    */
    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;

    /*
        Specifies if the alpha channel should be used for blending with other windows in the window system.
        You'll almost always want to simply ignore the alpha channel, hence VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
    */
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // Ignore obscured pixels to improve performance.

    /*
        It's possible that your swap chain becomes invalid or unoptimized while your application is running,
        for example because the window was resized. In that case the swap chain actually needs to be recreated
        from scratch and a reference to the old one must be specified in this field.
    */
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create swap chain.
    if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");

    // Also return image format and extent.
    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;
}
void VulkanExample::destroySwapchain()
{
    vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
}
void VulkanExample::retrieveSwapchainImages()
{
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapchainImages.data());
}
void VulkanExample::createImageViews()
{
    swapchainImageViews.resize(swapchainImages.size());

    // loop through swap chain images.
    for (size_t i = 0; i < swapchainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];

        /*
            The viewType and format fields specify how the image data should be interpreted.
            The viewType parameter allows you to treat images as 1D textures, 2D textures,
            3D textures and cube maps.
        */
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainImageFormat;

        /*
            The components field allows you to swizzle the color channels around.
            For example, you can map all of the channels to the red channel for a monochrome texture.
            You can also map constant values of 0 and 1 to a channel.

            Default mapping:
        */
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        /*
            The subresourceRange field describes what the image's purpose is
            and which part of the image should be accessed.
            Our images will be used as color targets without any mipmapping levels or multiple layers.
        */
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        // Create image view.
        if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create image views!");
    }
}
void VulkanExample::destroyImageViews()
{
    for (auto imageView : swapchainImageViews)
    {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }
}
void VulkanExample::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    /*
        VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment.
        VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start.
        VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don't care about them.
    */
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    /*
        VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later.
        VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation.
    */
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    /*
        Some of the most common layouts are:

        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment.
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain.
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation.
    */
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    /*
        Subpasses and attachment references.
        A single render pass can consist of multiple subpasses.
        Subpasses are subsequent rendering operations that depend on the contents
        of framebuffers in previous passes, for example a sequence of post-processing
        effects that are applied one after another. If you group these rendering
        operations into one render pass, then Vulkan is able to reorder the operations
        and conserve memory bandwidth for possibly better performance.
    */
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    /*
        The index of the attachment in this array is directly referenced from the
        fragment shader with the layout(location = 0) out vec4 outColor directive!
    */
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    /*
        Render pass create info.
    */
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // Subpass dependencies.
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // Create render pass.
    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass.");
}
void VulkanExample::destroyRenderPass()
{
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
}
VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice logicalDevice)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // cast char pointer to uint32_t pointer

    // Create shader module.
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}
void VulkanExample::createPipelineLayout()
{
    /*
        Pipeline layout.
        You can use uniform values in shaders, which are globals similar to dynamic
        state variables that can be changed at drawing time to alter the behavior of
        your shaders without having to recreate them. They are commonly used to pass
        the transformation matrix to the vertex shader, or to create texture samplers
        in the fragment shader.

        These uniform values need to be specified during pipeline creation by creating
        a VkPipelineLayout object. Even though we won't be using them until a future chapter,
        we are still required to create an empty pipeline layout.

        Create a class member to hold this object, because we'll refer to it from other
        functions at a later point in time:
    */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    // Create pipeline layout.
    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass.");
    }
}
void VulkanExample::destroyPipelineLayout()
{
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
}
void VulkanExample::createGraphicsPipeline()
{
    // Read shader file.
    auto vertShaderCode = Util::readFile("shader/vert.spv");
    auto fragShaderCode = Util::readFile("shader/frag.spv");

    // Create shader module with shader code.
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, logicalDevice);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, logicalDevice);

    /*
        Shader stage create info.

        To use the shaders we'll need to assign them to a specific
        pipeline stage through VkPipelineShaderStageCreateInfo structures
        as part of the actual pipeline creation process.
    */
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    /*
        Vertex input state create info.
        Describes the format of the vertex data that will be passed to the vertex shader.
        It describes this in roughly two ways:

        - Bindings: spacing between data and whether the data is per-vertex or per-instance.
        - Attribute descriptions: type of the attributes passed to the vertex shader,
                    which binding to load them from and at which offset.
    */
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    /*
        Input assembly state create info.
        Describes what kind of geometry will be drawn from the vertices
        and if primitive restart should be enabled.
        The former is specified in the topology member and can have values like:

        - VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices.
        - VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse.
        - VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line.
        - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse.
        - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle.
    */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    /*
        Viewport:
        A viewport describes the region of the framebuffer that the output will be rendered to.
    */
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchainExtent.width;
    viewport.height = (float)swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    /*
        Scissor:
        While viewports define the transformation from the image to the framebuffer,
        scissor rectangles define in which regions pixels will actually be stored.
        Any pixels outside the scissor rectangles will be discarded by the rasterizer.
        They function like a filter rather than a transformation.
    */
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchainExtent;

    /*
       Viewport state create info.
       Now this viewport and scissor rectangle need to be combined into
       a viewport state using the VkPipelineViewportStateCreateInfo struct.
       It is possible to use multiple viewports and scissor rectangles on
       some graphics cards, so its members reference an array of them.
       Using multiple requires enabling a GPU feature.
    */
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    /*
        Rasterization state create info.
        The rasterizer takes the geometry that is shaped by the vertices from the
        vertex shader and turns it into fragments to be colored by the fragment shader.
        It also performs depth testing, face culling and the scissor test,
        and it can be configured to output fragments that fill entire polygons
        or just the edges (wireframe rendering).
    */
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    /*
        Multi-sample state create info.
        The VkPipelineMultisampleStateCreateInfo struct configures multisampling,
        which is one of the ways to perform anti-aliasing.
    */
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    /*
        Color blending.
        After a fragment shader has returned a color,
        it needs to be combined with the color that is already in the framebuffer.
        This transformation is known as color blending and there are two ways to do it:

        - Mix the old and new value to produce a final color
        - Combine the old and new value using a bitwise operation
    */
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    /*
        Dynamic state
        A limited amount of the state that we've specified in the previous structs
        can actually be changed without recreating the pipeline.
        Examples are the size of the viewport, line width and blend constants.
        If you want to do that, then you'll have to fill in a VkPipelineDynamicStateCreateInfo structure.
    */
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    /*
        Graphics pipeline create info.
    */
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    // Create graphics pipeline.
    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // Destroy used shader modules.
    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}
void VulkanExample::destroyGraphicsPipeline()
{
    vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
}
void VulkanExample::createFramebuffers()
{
    swapchainFramebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++)
    {
        VkImageView attachments[] = {
            swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        // Create framebuffer.
        if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer.");
    }
}
void VulkanExample::freeCommandBuffers()
{
    vkFreeCommandBuffers(logicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}
void VulkanExample::destroyFramebuffers()
{
    for (auto framebuffer : swapchainFramebuffers)
    {
        vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
    }
}
void VulkanExample::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    /*
        Command pool create info.
        Command buffers are executed by submitting them on one of the device queues,
        like the graphics and presentation queues we retrieved. Each command pool can
        only allocate command buffers that are submitted on a single type of queue.
        We're going to record commands for drawing, which is why we've chosen the graphics queue family.

        There are two possible flags for command pools:

        - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT:
            Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
        - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT:
            Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
    */
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0; // Optional

    // Create command pool.
    if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool.");
    }
}
void VulkanExample::destroyCommandPool()
{
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
}
void VulkanExample::allocateCommandBuffers()
{
    commandBuffers.resize(swapchainFramebuffers.size());

    /*
        Command buffers are allocated with the vkAllocateCommandBuffers function,
        which takes a VkCommandBufferAllocateInfo struct as parameter that specifies
        the command pool and number of buffers to allocate.
    */
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    // Create command buffers.
    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers.");
    }

    /*
        Starting command buffer recording.

        We begin recording a command buffer by calling vkBeginCommandBuffer with
        a small VkCommandBufferBeginInfo structure as argument that specifies some
        details about the usage of this specific command buffer.
    */
    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording command buffer.");
        }
    }
}
void VulkanExample::beginRenderPass()
{
    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapchainFramebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapchainExtent;

        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end command buffer.");
        }
    }
}
void VulkanExample::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create synchronization objects for a frame.");
}
void VulkanExample::destroySyncObjects()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
    }
}

void VulkanExample::render()
{
    vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    // vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

    // Acquiring an image from the swap chain.
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(logicalDevice, swapchain, UINT64_MAX,
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swapchain image.");

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
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
        throw std::runtime_error("Failed to submit command buffer.");

    // Presentation.
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapchain();
    }
    else if (result != VK_SUCCESS)
        throw std::runtime_error("failed to present swap chain image!");

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
void VulkanExample::init(VkApplicationInfo appInfo)
{
    initWindow();
    initVulkan(appInfo);
    createSurface();
    selectPhysicalDevice();
    createLogicalDevice();
    initSwapchain();
    createSyncObjects();
}
void VulkanExample::cleanup()
{
    waitIdle();
    destroySyncObjects();
    cleanupSwapchain();
    destroyCommandPool();
    destroyLogicalDevice();
    destroySurface();
    destroyVulkanInstance();
    terminateWindow();
}
void VulkanExample::initSwapchain()
{
    createSwapchain();
    retrieveSwapchainImages();
    createImageViews();
    createRenderPass();
    createPipelineLayout();
    createGraphicsPipeline();
    createFramebuffers();
    if (commandPool == VK_NULL_HANDLE)
        createCommandPool();
    allocateCommandBuffers();
    beginRenderPass();
}
void VulkanExample::cleanupSwapchain()
{
    destroyFramebuffers();
    freeCommandBuffers();
    destroyGraphicsPipeline();
    destroyPipelineLayout();
    destroyRenderPass();
    destroyImageViews();
    destroySwapchain();
}
void VulkanExample::recreateSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    waitIdle();
    cleanupSwapchain();
    initSwapchain();
}
