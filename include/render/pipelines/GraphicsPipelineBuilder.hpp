#pragma once

#include <render/pipelines/PipelineBuilder.hpp>
#include <render/pipelines/PipelineDesc.hpp>
#include <gfx/Device.hpp>
#include <scene/Geometry.hpp>

#include <vulkan/vulkan.h>



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
