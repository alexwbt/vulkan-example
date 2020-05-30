#include "vulkan_example.h"

#include <set>

namespace VulkanExample
{

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        // Get number of supported extensions from device.
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        // Get properties of supported extensions from device.
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // Clone list of required extensions
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        // Check if all required extensions are listed in availableExtensions.
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        // Check which queue families(that supports the commands that we want to use) are supperted.
        QueueFamilyIndices indices = findQueueFamilies(device, surface); // queue.cpp

        // Check if physical device(GPU) supports reqruied extensions.
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        return indices.isComplete() && extensionsSupported;
    }

    VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
    {
        // VkPhysicalDevice object will be implicitly destroyed when the VkInstance is destroyed
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); // Get number of available devices.
        if (deviceCount == 0)
        {
            // Throw error if there are no available device.
            throw std::runtime_error("Failed to find GPU with Vulkan support.");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()); // Get all available devices.

        // Select first suitable device.
        for (const auto& device : devices) {
            if (isDeviceSuitable(device, surface)) {
                physicalDevice = device;
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE) {
            // Throw error if there are no suitable device.
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        return physicalDevice;
    }

}
