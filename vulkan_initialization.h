#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

VkInstance initVulkan(VkApplicationInfo info);
void destroyVulkanInstance(VkInstance instance);
