#include "vulkan_example.h"

#include "util.h"

#include <stdexcept>

namespace VulkanExample
{

    void createGraphicsPipeline(VkDevice logicalDevice, VkExtent2D swapchainExtent)
    {
        // Read shader file.
        auto vertShaderCode = Util::readFile("vert.spv");
        auto fragShaderCode = Util::readFile("frag.spv");

        // Create shader module with shader code.
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, logicalDevice);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, logicalDevice);

        /*
            Shader stage create info.

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

        /*
            Vertex input state create info.
            Describes the format of the vertex data that will be passed to the vertex shader.
            It describes this in roughly two ways:

            - Bindings: spacing between data and whether the data is per-vertex or per-instance.
            - Attribute descriptions: type of the attributes passed to the vertex shader,
                        which binding to load them from and at which offset.
        */
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

        /*
            Input assembly state create info.
            Describes what kind of geometry will be drawn from the vertices
            and if primitive restart should be enabled.
            The former is specified in the topology member and can have values like:

            - VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices.
            - VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse.
            - VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line.
            - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse.
            - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle.
        */
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        /*
            Viewport:
            A viewport describes the region of the framebuffer that the output will be rendered to.
        */
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapchainExtent.width;
        viewport.height = (float)swapchainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        /*
            Scissor:
            While viewports define the transformation from the image to the framebuffer,
            scissor rectangles define in which regions pixels will actually be stored.
            Any pixels outside the scissor rectangles will be discarded by the rasterizer.
            They function like a filter rather than a transformation.
        */
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapchainExtent;

        /*
           Viewport state create info.
           Now this viewport and scissor rectangle need to be combined into
           a viewport state using the VkPipelineViewportStateCreateInfo struct.
           It is possible to use multiple viewports and scissor rectangles on
           some graphics cards, so its members reference an array of them.
           Using multiple requires enabling a GPU feature.
        */
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        /*
            Rasterization state create info.

        */

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
        if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

}
