#pragma once
#include <vector>
#include <string>
#include <variant>
#include <vulkan/vulkan.h>

namespace vks
{
    enum class PipelineType
    {
        Graphics,
        Compute,
        Custom
    };

    struct GraphicsPipelineDesc
    {
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        uint32_t subpass = 0;

        std::string vertexShader;
        std::string fragmentShader;

        VkExtent2D viewportExtent{};

        VkCullModeFlags cull = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkCompareOp depthCompare = VK_COMPARE_OP_LESS;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        bool depthTest = true;
        bool depthWrite = true;
        bool alphaBlending = false;
        bool isVertexInput = true; // Whether pipeline has vertex input (for procedural pipelines)
    };

    struct PipelineDesc
    {
        PipelineType type;

        std::vector<VkDescriptorSetLayout> setLayouts;
        std::vector<VkPushConstantRange> pushConstants;

        std::variant<
            GraphicsPipelineDesc
        > payload;
    };
}
