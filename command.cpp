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

}
