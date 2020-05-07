#include "layer.h"

#include <vulkan/vulkan.h>

bool checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // Get number of available layers.

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());  // Get properties of available layers.

	// Check if all of the layers in validationLayers exist in the availableLayers list.
	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}
