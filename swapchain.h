#pragma once

#include <vulkan/vulkan.h>

VkSwapchainKHR createSwapchain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface);
void destroySwapchain(VkDevice logicalDevice, VkSwapchainKHR swapchain);
