#include "queue.h"

#include <vector>

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // Get number of available queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); // Get properties of available queue families.

    /*
        VkQueueFamilyProperties contains details of the queue family,
        including the type of operations that are supported
        and the number of queues that can be created based on that family.
        We need to find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT.
    */
    for (int i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
            break;
        }
    }

    return indices;
}
