#include "image.h"

/*
    Revrieve handles of VkImages from swap chain.

    The images were created by the implementation
    for the swap chain and they will be automatically
    cleaned up once the swap chain has been destroyed.
*/
std::vector<VkImage> retrieveSwapchainImages(VkDevice logicalDevice, VkSwapchainKHR swapchain)
{
    std::vector<VkImage> swapchainImages;

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapchainImages.data());

    return swapchainImages;
}
