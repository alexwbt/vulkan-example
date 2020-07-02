#include "vulkan_example.h"
#include "util.h"

namespace VulkanExample
{

    /*
        We need to tell Vulkan about the framebuffer attachments that will
        be used while rendering.
        We need to specify how many color and depth buffers there will be,
        how many samples to use for each of them and how their contents
        should be handled throughout the rendering operations.
        All of this information is wrapped in a render pass object.
    */
    #pragma warning (disable : 26812)
    VkRenderPass createRenderPass(VkDevice logicalDevice, VkFormat swapchainImageFormat)
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        /*
            VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment.
            VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start.
            VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don't care about them.
        */
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

        /*
            VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later.
            VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation.
        */
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        /*
            Some of the most common layouts are:

            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment.
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain.
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation.
        */
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        /*
            Subpasses and attachment references.
            A single render pass can consist of multiple subpasses.
            Subpasses are subsequent rendering operations that depend on the contents
            of framebuffers in previous passes, for example a sequence of post-processing
            effects that are applied one after another. If you group these rendering
            operations into one render pass, then Vulkan is able to reorder the operations
            and conserve memory bandwidth for possibly better performance. 
        */
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        /*
            The index of the attachment in this array is directly referenced from the
            fragment shader with the layout(location = 0) out vec4 outColor directive!
        */
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        /*
            Render pass create info.
        */
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        // Subpass dependencies.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        // Create render pass.
        VkRenderPass renderPass;
        if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create render pass.");
        }

        return renderPass;
    }

    void destroyRenderPass(VkDevice logicalDevice, VkRenderPass renderPass)
    {
        vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
    }

    VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice logicalDevice)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // cast char pointer to uint32_t pointer

        // Create shader module.
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkPipelineLayout createPipelineLayout(VkDevice logicalDevice)
    {
        /*
            Pipeline layout.
            You can use uniform values in shaders, which are globals similar to dynamic
            state variables that can be changed at drawing time to alter the behavior of
            your shaders without having to recreate them. They are commonly used to pass
            the transformation matrix to the vertex shader, or to create texture samplers
            in the fragment shader.

            These uniform values need to be specified during pipeline creation by creating
            a VkPipelineLayout object. Even though we won't be using them until a future chapter,
            we are still required to create an empty pipeline layout.

            Create a class member to hold this object, because we'll refer to it from other
            functions at a later point in time:
        */
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        // Create pipeline layout.
        VkPipelineLayout pipelineLayout;
        if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass.");
        }

        return pipelineLayout;
    }

    void destroyPipelineLayout(VkDevice logicalDevice, VkPipelineLayout pipelineLayout)
    {
        vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    }

    VkPipeline createGraphicsPipeline(VkDevice logicalDevice, VkExtent2D swapchainExtent, VkRenderPass renderPass, VkPipelineLayout pipelineLayout)
    {
        // Read shader file.
        auto vertShaderCode = Util::readFile("shader/vert.spv");
        auto fragShaderCode = Util::readFile("shader/frag.spv");

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
            The rasterizer takes the geometry that is shaped by the vertices from the
            vertex shader and turns it into fragments to be colored by the fragment shader.
            It also performs depth testing, face culling and the scissor test,
            and it can be configured to output fragments that fill entire polygons
            or just the edges (wireframe rendering).
        */
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        /*
            Multi-sample state create info.
            The VkPipelineMultisampleStateCreateInfo struct configures multisampling,
            which is one of the ways to perform anti-aliasing.
        */
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        /*
            Color blending.
            After a fragment shader has returned a color,
            it needs to be combined with the color that is already in the framebuffer.
            This transformation is known as color blending and there are two ways to do it:

            - Mix the old and new value to produce a final color
            - Combine the old and new value using a bitwise operation
        */
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        /*
            Dynamic state
            A limited amount of the state that we've specified in the previous structs
            can actually be changed without recreating the pipeline.
            Examples are the size of the viewport, line width and blend constants.
            If you want to do that, then you'll have to fill in a VkPipelineDynamicStateCreateInfo structure.
        */
        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        /*
            Graphics pipeline create info.
        */
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;

        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional

        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        // Create graphics pipeline.
        VkPipeline graphicsPipeline;
        if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        // Destroy used shader modules.
        vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);

        return graphicsPipeline;
    }

    void destroyGraphicsPipeline(VkDevice logicalDevice, VkPipeline graphicsPipeline)
    {
        vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
    }

}
