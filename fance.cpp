#include "vulkan_example.h"

namespace VulkanExample
{

    std::vector<VkFence> createFences(VkDevice logicalDevice, size_t size)
    {
        std::vector<VkFence> fences(size);

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < size; i++)
        {
            if (vkCreateFence(logicalDevice, &fenceInfo, nullptr, &fences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create fences.");
            }
        }
        
        return fences;
    }

    void destroyFences(VkDevice logicalDevice, std::vector<VkFence> fences)
    {
        for (size_t i = 0; i < fences.size(); i++)
        {
            vkDestroyFence(logicalDevice, fences[i], nullptr);
        }
    }

}
