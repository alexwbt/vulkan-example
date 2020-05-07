#include "vulkan_initialization.h"

#include <stdexcept>
#include <vector>
#include <iostream>

VkInstance initVulkan(VkApplicationInfo info)
{
	/*
		CreateInfo tells the Vulkan driver which global extensions
		and validation layers to use. (not optional)
	*/
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &info;

	// Enable global extensions.
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // GLFW built-in function: returns required extensions.
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	// Enable global validation layers.
	createInfo.enabledLayerCount = 0;

	/*
		Check extension support.
	*/
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr); // Get number of extensions.
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()); // Get extension properties.
	// (optional first parameter to filter extensions by validation layer)

	// Listing supported extensions
	std::cout << "available extensions:\n";
	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << '\n';
	}

	/*
		Create Vulkan instance.
	*/
	VkInstance instance;
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance.");
	}

	return instance;
}

void destroyVulkanInstance(VkInstance instance)
{
	vkDestroyInstance(instance, nullptr);
}
