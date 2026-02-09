#pragma once

#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include <render/pipelines/PipelineDesc.hpp>

namespace vks
{
    class Device;

    class PipelineManager
    {
    public:
        PipelineManager(const vks::Device& device);

        void createOrReplace(
            const std::string& name,
            const PipelineDesc& desc
        );

        void destroy(const std::string& name);
        void recreate(const std::string& name);
        void recreateAll();

        VkPipeline getPipeline(const std::string& name) const;
        VkPipelineLayout getLayout(const std::string& name) const;

    private:
        struct Entry
        {
            PipelineDesc desc;
            VkPipeline pipeline = VK_NULL_HANDLE;
            VkPipelineLayout layout = VK_NULL_HANDLE;
        };

        void buildPipeline(Entry& entry);

        const vks::Device& m_device;
        VkPipelineCache m_cache = VK_NULL_HANDLE;

        std::unordered_map<std::string, Entry> m_pipelines;
    };
}
