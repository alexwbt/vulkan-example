#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window);
void destroySurface(VkInstance instance, VkSurfaceKHR surface);
