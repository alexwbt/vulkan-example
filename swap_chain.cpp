#include "swap_chain.h"

/*
    Swap chain is essentially a queue of images that are waiting to be presented to the screen. (framebuffer)
    The general purpose of swap chain is to synchronize the presentation of images with the refresh rate of the screen.

    Not all graphics cards are capable of presenting images directly to a screen.
    And since image presentation is heavily tied into the window system and the surfaces associated with windows,
    it is not part of the Vulkan core. You have to enable the VK_KHR_swapchain device extension after querying for its support.

    Besides from availability, there are three more properties we need to check:
    - Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
    - Surface formats (pixel format, color space)
    - Available presentation modes
*/
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

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

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{

}
