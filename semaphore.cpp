#include "vulkan_example.h"

namespace VulkanExample
{

    VkSemaphore createSemaphore(VkDevice logicalDevice)
    {
        VkSemaphore semaphore;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create semaphore.");
        }

        return semaphore;
    }

    void destroySemaphore(VkDevice logicalDevice, VkSemaphore semaphore)
    {
        vkDestroySemaphore(logicalDevice, semaphore, nullptr);
    }

}
