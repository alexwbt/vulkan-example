#include "vulkan_example.h"

namespace VulkanExample
{

    VkCommandPool createCommandPool(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

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
        VkCommandPool commandPool;
        if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool.");
        }

        return commandPool;
    }

    void destroyCommandPool(VkDevice logicalDevice, VkCommandPool commandPool)
    {
        vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
    }

    std::vector<VkCommandBuffer> allocateCommandBuffers(VkDevice logicalDevice, VkCommandPool commandPool, size_t size)
    {
        std::vector<VkCommandBuffer> commandBuffers(size);

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

        return commandBuffers;
    }

    /*
        Starting a render pass.

        Drawing starts by beginning the render pass with vkCmdBeginRenderPass.
        The render pass is configured using some parameters in a VkRenderPassBeginInfo struct.
    */
    void beginRenderPass(VkRenderPass renderPass, VkExtent2D swapchainExtent, VkPipeline graphicsPipeline,
        std::vector<VkCommandBuffer> commandBuffers, std::vector<VkFramebuffer> swapchainFramebuffers)
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

}
