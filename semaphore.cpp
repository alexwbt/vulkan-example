#include "vulkan_example.h"

namespace VulkanExample
{

    std::vector<VkSemaphore> createSemaphores(VkDevice logicalDevice, size_t size)
    {
        std::vector<VkSemaphore> semaphores(size);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (size_t i = 0; i < size; i++)
        {
            if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &semaphores[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create semaphores.");
            }
        }

        return semaphores;
    }

    void destroySemaphores(VkDevice logicalDevice, std::vector<VkSemaphore> semaphores)
    {
        for (size_t i = 0; i < semaphores.size(); i++)
        {
            vkDestroySemaphore(logicalDevice, semaphores[i], nullptr);
        }
    }

}
