#include "device.h"

#include <stdexcept>
#include <vector>

#include "queue.h"

bool isDeviceSuitable(VkPhysicalDevice device)
{
	// Check which queue families(that supports the commands that we want to use) are supperted
	// queue.cpp
	QueueFamilyIndices indices = findQueueFamilies(device);
	return indices.isComplete();
}

VkPhysicalDevice selectPhysicalDevice(VkInstance instance)
{
	// VkPhysicalDevice object will be implicitly destroyed when the VkInstance is destroyed
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); // Get number of available devices.
	if (deviceCount == 0)
	{ 
		// Throw error if there are no available device.
		throw std::runtime_error("Failed to find GPU with Vulkan support.");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()); // Get all available devices.

	// Select first suitable device.
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE) {
		// Throw error if there are no suitable device.
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	return physicalDevice;
}
