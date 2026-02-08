#include <fstream>
#include <memory>
#include <stdexcept>
#include <vks/PipelineManager.hpp>

#include "vks/Device.hpp"
#include "vks/Render/GraphicsPipelineBuilder.hpp"

namespace vks
{
    PipelineManager::PipelineManager(const Device& device) : m_device(device)
    {
        // Create pipeline cache
        VkPipelineCacheCreateInfo cacheInfo{};
        cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkCreatePipelineCache(m_device.logical(), &cacheInfo, nullptr, &m_cache);
    }

    void PipelineManager::createOrReplace(
        const std::string& name,
        const PipelineDesc& desc
    )
    {
        if (m_pipelines.find(name) != m_pipelines.end())
            destroy(name);

        Entry entry{};
        entry.desc = desc;

        buildPipeline(entry);
        m_pipelines[name] = entry;
    }

    void PipelineManager::destroy(const std::string& name)
    {
        auto it = m_pipelines.find(name);
        if (it == m_pipelines.end())
            throw std::runtime_error("Pipeline not found: " + name);

        vkDestroyPipeline(m_device.logical(), it->second.pipeline, nullptr);
        vkDestroyPipelineLayout(m_device.logical(), it->second.layout, nullptr);
        m_pipelines.erase(it);
    }

    void PipelineManager::recreate(const std::string& name)
    {
        auto it = m_pipelines.find(name);
        if (it == m_pipelines.end())
            throw std::runtime_error("Pipeline not found: " + name);

        destroy(name);
        buildPipeline(it->second);
    }

    void PipelineManager::recreateAll()
    {
        // store the current pipelines
        std::vector<std::pair<std::string, Entry>> oldPipelines(m_pipelines.begin(), m_pipelines.end());
        for (auto& pair : oldPipelines)
            destroy(pair.first);

        // Rebuild all pipelines
        for (auto& pair : oldPipelines)
        {
            buildPipeline(pair.second);
            m_pipelines[pair.first] = pair.second;
        }
    }

    VkPipeline PipelineManager::getPipeline(const std::string& name) const
    {
        auto it = m_pipelines.find(name);
        if (it == m_pipelines.end())
            throw std::runtime_error("Pipeline not found: " + name);

        return it->second.pipeline;
    }

    VkPipelineLayout PipelineManager::getLayout(const std::string& name) const
    {
        auto it = m_pipelines.find(name);
        if (it == m_pipelines.end())
            throw std::runtime_error("Pipeline not found: " + name);

        return it->second.layout;
    }

    void PipelineManager::buildPipeline(Entry& entry)
    {
        // Create pipeline layout
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = entry.desc.setLayouts.size();
        layoutInfo.pSetLayouts = entry.desc.setLayouts.data();
        layoutInfo.pushConstantRangeCount = entry.desc.pushConstants.size();
        layoutInfo.pPushConstantRanges = entry.desc.pushConstants.data();

        vkCreatePipelineLayout(
            m_device.logical(),
            &layoutInfo,
            nullptr,
            &entry.layout
        );

        // Dispatch by pipeline type
        std::unique_ptr<IPipelineBuilder> builder;

        if (entry.desc.type == PipelineType::Graphics)
        {
            auto& g = std::get<GraphicsPipelineDesc>(entry.desc.payload);

        builder = std::make_unique<GraphicsPipelineBuilder>(
                m_device,
                g
            );

            entry.pipeline = builder->build(entry.layout, m_cache);
        }
        else if (entry.desc.type == PipelineType::Compute)
        {
            // Not Implemented but can be added later
        }
    }
}
