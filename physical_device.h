#pragma once

#include <vulkan/vulkan.h>
#include <vector>

// List of required extensions.
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkPhysicalDevice selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
