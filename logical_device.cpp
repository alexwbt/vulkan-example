#include "vulkan_example.h"

#include <stdexcept>
#include <set>

namespace VulkanExample
{

    VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue* graphicsQueue, VkQueue* presentQueue)
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

        // Create multiple queue families that are necessary for the required queue.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // List of create info struct.
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() }; // required queue.
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            // Specify number of required queues(to be created) for a single family
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
            queueCreateInfo.queueCount = 1;
            // Assign priorities to influence the scheduling of command buffer execution.
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Specify required device features. (e.g. geometry shaders)
        // Empty for now
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Create logical device.
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
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

        // Enable requied extensions.
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()); // physical_device.h
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // Enable validation layers. (in debug mode)
        if (enableValidationLayers) // layer.h
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()); // layer.h
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
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, presentQueue);

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

}
