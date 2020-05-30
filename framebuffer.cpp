#include "vulkan_example.h"

namespace VulkanExample
{

    /*
        The attachments specified during render pass creation are bound by wrapping them into a VkFramebuffer object.
        A framebuffer object references all of the VkImageView objects that represent the attachments.
        In our case that will be only a single one: the color attachment.
        However, the image that we have to use for the attachment depends
        on which image the swap chain returns when we retrieve one for presentation.
        That means that we have to create a framebuffer for all of the images in the
        swap chain and use the one that corresponds to the retrieved image at drawing time.
    */
    std::vector<VkFramebuffer> createFramebuffers(VkDevice logicalDevice, std::vector<VkImageView> swapchainImageViews, VkRenderPass renderPass, VkExtent2D swapchainExtent)
    {
        std::vector<VkFramebuffer> swapchainFramebuffers(swapchainImageViews.size());

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
            {
                throw std::runtime_error("Failed to create framebuffer.");
            }
        }

        return swapchainFramebuffers;
    }

    void destroyFramebuffers(VkDevice logicalDevice, std::vector<VkFramebuffer> swapchainFramebuffers)
    {
        for (auto framebuffer : swapchainFramebuffers)
        {
            vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
        }
    }

}
