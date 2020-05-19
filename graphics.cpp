#include "vulkan_example.h"

#include "util.h"

#include <stdexcept>

namespace VulkanExample
{

    void createGraphicsPipeline(VkDevice logicalDevice)
    {
        // Read shader file.
        auto vertShaderCode = Util::readFile("vert.spv");
        auto fragShaderCode = Util::readFile("frag.spv");

        // Create shader module with shader code.
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, logicalDevice);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, logicalDevice);
        
        /*
            To use the shaders we'll need to assign them to a specific
            pipeline stage through VkPipelineShaderStageCreateInfo structures
            as part of the actual pipeline creation process.
        */
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // Destroy used shader modules.
        vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
    }

    VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice logicalDevice)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // cast char pointer to uint32_t pointer

        // Create shader module.
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(logicalDevice , &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

}
