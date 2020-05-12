#pragma once

#include <vulkan/vulkan.h>

#include "queue.h"

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueue* graphicsQueue, VkQueue* presentQueue);
void destroyLogicalDevice(VkDevice device);
