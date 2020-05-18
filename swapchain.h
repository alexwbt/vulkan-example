#pragma once

#include <vulkan/vulkan.h>

VkSwapchainKHR createSwapchain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, VkFormat* swapchainImageFormat, VkExtent2D* swapchainExtent);
void destroySwapchain(VkDevice logicalDevice, VkSwapchainKHR swapchain);
