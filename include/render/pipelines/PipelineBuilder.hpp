#pragma once

#include <vulkan/vulkan.h>

namespace vks
{
    class IPipelineBuilder
    {
    public:
        virtual ~IPipelineBuilder() = default;

        virtual VkPipeline build(
            VkPipelineLayout layout,
            VkPipelineCache cache
        ) = 0;
    };
}
