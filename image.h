#pragma once

#include <vector>
#include <vulkan/vulkan.h>

std::vector<VkImage> retrieveSwapchainImages(VkDevice logicalDevice, VkSwapchainKHR swapchain);
