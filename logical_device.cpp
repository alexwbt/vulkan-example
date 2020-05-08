#include "logical_device.h"

#include "layer.h"

#include <stdexcept>

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkQueue *graphicsQueue)
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    // Specify number of required queues(to be created) for a single family
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;

    // Assign priorities to influence the scheduling of command buffer execution.
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // Specify required device features. (e.g. geometry shaders)
    // Empty for now
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Create logical device with above two struct.
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    /*
        The remainder of the information bears a resemblance to
        the VkInstanceCreateInfo struct and requires you to specify
        extensions and validation layers. The difference is that
        these are device specific this time.

        An example of a device specific extension is VK_KHR_swapchain,
        which allows you to present rendered images from that device to windows.
        It is possible that there are Vulkan devices in the system that
        lack this ability, for example because they only support compute operations.

        Previous implementations of Vulkan made a distinction between
        instance and device specific validation layers, but this is no longer the case.
        That means that the enabledLayerCount and ppEnabledLayerNames fields
        of VkDeviceCreateInfo are ignored by up-to-date implementations.
        However, it is still a good idea to set them anyway to be
        compatible with older implementations:
    */
    createInfo.enabledExtensionCount = 0;

    if (enableValidationLayers) // layer.h
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    // Create logical device.
    VkDevice device;
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device.");
    }

    // Retrieve queue handles for each queue family.
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, graphicsQueue);

    return device;
}

void destroyLogicalDevice(VkDevice device)
{
    /*
        Logical devices don't interact directly with instances,
        which is why it's not included as a parameter.
    */
    vkDestroyDevice(device, nullptr);
}
