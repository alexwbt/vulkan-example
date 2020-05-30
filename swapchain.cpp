#include "vulkan_example.h"

#include <algorithm>

namespace VulkanExample
{

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /*
        Swap chain is essentially a queue of images that are waiting to be presented to the screen.
        The general purpose of swap chain is to synchronize the presentation of images with the refresh rate of the screen.

        Not all graphics cards are capable of presenting images directly to a screen.
        And since image presentation is heavily tied into the window system and the surfaces associated with windows,
        it is not part of the Vulkan core. You have to enable the VK_KHR_swapchain device extension after querying for its support.

        Besides from availability, there are three more properties we need to check:
        - Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
        - Surface formats (pixel format, color space)
        - Available presentation modes
    */
    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        SwapchainSupportDetails details;

        // Get basic surface capabilities.
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // Get supported surface formats.
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        // Get supported present modes.
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    /*
        To find the best settings for the swap chain.
        There are three types of settings to determine:
        - Surface format (color depth)
        - Presentation mode (conditions for "swapping" images to the screen)
        - Swap extent (resolution of images in swap chain)
    */

    // Choose swap chain surface format (color depth)
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

    /*
        Presentation mode represents the conditions for showing images to the screen.
        There are four possible modes available in Vulkan :
        - VK_PRESENT_MODE_IMMEDIATE_KHR: Rendered images transfer to the screen right away.
        - VK_PRESENT_MODE_FIFO_KHR: Stores rendered images to a queue. If the queue is full, the program has to wait.
        - VK_PRESENT_MODE_FIFO_RELAXED_KHR: When the queue is empty, the image is transferred right away instead of waiting. May result un visible tearing.
        - VK_PRESENT_MODE_MAILBOX_KHR: When the queue is full, the images is queue are replaced with newer ones instead of blocking the application.
    */
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

    /*
        Swap extent is the resolution of the swap change images.
        Always equal to the resolution of the presentation window.
        The range of the possible resolutions is defined in VkSurfaceCapabilitiesKHR.
    */
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = { WIDTH, HEIGHT };

            // Clamp WIDTH and HEIGHT to be supported by the implementation.
            actualExtent.width = std::max(capabilities.minImageExtent.width,
                std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height,
                std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    // Create swap chain.
    VkSwapchainKHR createSwapchain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, VkFormat* swapchainImageFormat, VkExtent2D* swapchainExtent)
    {
        // Get supported format, present mode and extent.
        SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice, surface);

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
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
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
        VkSwapchainKHR swapChain;
        if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        // Also return image format and extent.
        *swapchainImageFormat = surfaceFormat.format;
        *swapchainExtent = extent;

        return swapChain;
    }

    void destroySwapchain(VkDevice logicalDevice, VkSwapchainKHR swapchain)
    {
        vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
    }

}
