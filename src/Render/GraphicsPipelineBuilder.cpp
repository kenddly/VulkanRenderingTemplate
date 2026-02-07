#include <fstream>
#include <stdexcept>
#include <vks/Render/GraphicsPipelineBuilder.hpp>

namespace vks
{
    VkPipeline GraphicsPipelineBuilder::build(VkPipelineLayout layout, VkPipelineCache cache)
    {
        // ==============================
        // Shader stages
        // ==============================
        VkPipelineShaderStageCreateInfo stages[2]{};

        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = createShaderModuleFromFile(m_desc.vertexShader);
        stages[0].pName = "main";

        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = createShaderModuleFromFile(m_desc.fragmentShader);
        stages[1].pName = "main";

        // ==============================
        // Vertex input
        // ==============================
        auto bindingDescription = geometry::Vertex::getBindingDescription();
        auto attributeDescriptions = geometry::Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        if (m_desc.isVertexInput)
        {
            vertexInput.vertexBindingDescriptionCount = 1;
            vertexInput.pVertexBindingDescriptions = &bindingDescription;
            vertexInput.vertexAttributeDescriptionCount = attributeDescriptions.size();
            vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();
        } else
        {
            vertexInput.vertexBindingDescriptionCount = 0;
            vertexInput.pVertexBindingDescriptions = nullptr;
            vertexInput.vertexAttributeDescriptionCount = 0;
            vertexInput.pVertexAttributeDescriptions = nullptr;
        }

        // ==============================
        // Input assembly
        // ==============================
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = m_desc.topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // ==============================
        // Viewport + Scissor (dynamic)
        // ==============================
        VkPipelineViewportStateCreateInfo viewport{};
        viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport.viewportCount = 1;
        viewport.scissorCount = 1;

        VkPipelineDynamicStateCreateInfo dynamic{};
        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic.dynamicStateCount =
            static_cast<uint32_t>(m_desc.dynamicStates.size());
        dynamic.pDynamicStates = m_desc.dynamicStates.data();

        // ==============================
        // Rasterizer
        // ==============================
        VkPipelineRasterizationStateCreateInfo raster{};
        raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        raster.depthClampEnable = VK_FALSE;
        raster.rasterizerDiscardEnable = VK_FALSE;
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.lineWidth = 1.0f;
        raster.cullMode = m_desc.cull;
        raster.frontFace = m_desc.frontFace;
        raster.depthBiasEnable = VK_FALSE;

        // ==============================
        // Multisampling
        // ==============================
        VkPipelineMultisampleStateCreateInfo msaa{};
        msaa.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        msaa.sampleShadingEnable = VK_FALSE;

        // ==============================
        // Depth / Stencil
        // ==============================
        VkPipelineDepthStencilStateCreateInfo depth{};
        depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth.depthTestEnable = m_desc.depthTest ? VK_TRUE : VK_FALSE;
        depth.depthWriteEnable = m_desc.depthWrite ? VK_TRUE : VK_FALSE;
        depth.depthCompareOp = m_desc.depthCompare;
        depth.depthBoundsTestEnable = VK_FALSE;
        depth.stencilTestEnable = VK_FALSE;

        // ==============================
        // Color blending
        // ==============================
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

        if (m_desc.alphaBlending)
        {
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }
        else
        {
            colorBlendAttachment.blendEnable = VK_FALSE;
        }

        VkPipelineColorBlendStateCreateInfo blend{};
        blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend.logicOpEnable = VK_FALSE;
        blend.attachmentCount = 1;
        blend.pAttachments = &colorBlendAttachment;

        // ==============================
        // Final pipeline create info
        // ==============================
        VkGraphicsPipelineCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        info.stageCount = 2;
        info.pStages = stages;

        info.pVertexInputState = &vertexInput;
        info.pInputAssemblyState = &inputAssembly;
        info.pViewportState = &viewport;
        info.pRasterizationState = &raster;
        info.pMultisampleState = &msaa;
        info.pDepthStencilState = &depth;
        info.pColorBlendState = &blend;
        info.pDynamicState = &dynamic;

        info.layout = layout;
        info.renderPass = m_desc.renderPass;
        info.subpass = m_desc.subpass;

        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(
            m_device.logical(),
            cache,
            1,
            &info,
            nullptr,
            &pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create graphics pipeline");
        }

        vkDestroyShaderModule(m_device.logical(), stages[0].module, nullptr);
        vkDestroyShaderModule(m_device.logical(), stages[1].module, nullptr);

        return pipeline;
    }

    VkShaderModule GraphicsPipelineBuilder::createShaderModuleFromFile(const std::string& path) const
    {
        auto code = loadSpirv(path);

        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = code.size() * sizeof(uint32_t);
        info.pCode = code.data();

        VkShaderModule module;
        if (vkCreateShaderModule(m_device.logical(), &info, nullptr, &module) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module from file: " + path);
        }

        return module;
    }

    std::vector<uint32_t> GraphicsPipelineBuilder::loadSpirv(const std::string& path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Failed to open SPIR-V file: " + path);

        size_t size = file.tellg();
        std::vector<uint32_t> buffer(size / sizeof(uint32_t));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        file.close();

        return buffer;
    }
}
