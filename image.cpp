#include "vulkan_example.h"

namespace VulkanExample
{

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

    /*
        VkImageView is required to use any VkImage.
        It describes how to access the image and which part of
        the image to access, for example if it should be treated
        as a 2D texture depth texture without any mipmapping levels.

        An image view is sufficient to start using an image as a texture,
        but it's not quite ready to be used as a render target just yet.
    */
    std::vector<VkImageView> createImageViews(std::vector<VkImage> images, VkFormat imageFormat, VkDevice logicalDevice)
    {
        std::vector<VkImageView> imageViews(images.size());

        // loop through swap chain images.
        for (size_t i = 0; i < images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = images[i];

            /*
                The viewType and format fields specify how the image data should be interpreted.
                The viewType parameter allows you to treat images as 1D textures, 2D textures,
                3D textures and cube maps.
            */
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = imageFormat;

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
            if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }

        return imageViews;
    }

    // Destroy image views.
    void destroyImageViews(VkDevice logicalDevice, std::vector<VkImageView> imageViews)
    {
        for (auto imageView : imageViews)
        {
            vkDestroyImageView(logicalDevice, imageView, nullptr);
        }
    }

}
