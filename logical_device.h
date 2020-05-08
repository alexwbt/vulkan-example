#pragma once

#include <vulkan/vulkan.h>

#include "queue.h"

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkQueue *graphicsQueue);
void destroyLogicalDevice(VkDevice device);
