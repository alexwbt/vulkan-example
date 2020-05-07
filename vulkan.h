#pragma once

#include <vulkan/vulkan.h>

VkInstance initVulkan(VkApplicationInfo info);
void destroyVulkanInstance(VkInstance instance);

VkApplicationInfo createHelloTriangleAppInfo();
