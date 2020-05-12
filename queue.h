#pragma once

#include <vulkan/vulkan.h>

#include <optional>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily; // For rendering (drawing commands).
    std::optional<uint32_t> presentFamily; // For presetning render to window surface.

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
