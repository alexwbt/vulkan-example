#pragma once

#include <vector>
#include <vulkan/vulkan.h>

std::vector<VkImage> retrieveSwapchainImages(VkDevice logicalDevice, VkSwapchainKHR swapchain);

std::vector<VkImageView> createImageViews(std::vector<VkImage> images, VkFormat imageFormat, VkDevice logicalDevice);
void destroyImageViews(VkDevice logicalDevice, std::vector<VkImageView> imageViews);
