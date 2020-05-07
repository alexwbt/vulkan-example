#pragma once

#include <vulkan/vulkan.h>

struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
