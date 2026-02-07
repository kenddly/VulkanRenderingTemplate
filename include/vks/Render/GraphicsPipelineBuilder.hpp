#pragma once

#include <array>
#include <vks/Render/PipelineBuilder.hpp>
#include <vks/Render/PipelineDesc.hpp>
#include <vks/Device.hpp>
#include <vulkan/vulkan.h>

#include "vks/Geometry.hpp"


namespace vks
{
    class GraphicsPipelineBuilder final : public IPipelineBuilder
    {
    public:
        GraphicsPipelineBuilder(
            const Device& device,
            const GraphicsPipelineDesc& desc
        ) : m_device(device), m_desc(desc) {}

        VkPipeline build(VkPipelineLayout layout, VkPipelineCache cache) override;

    private:
        const Device& m_device;
        const GraphicsPipelineDesc& m_desc;

        static std::vector<uint32_t> loadSpirv(const std::string& path);

        VkShaderModule createShaderModuleFromFile(const std::string& path) const;
    };

}
