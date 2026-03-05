#pragma once
#include <vulkan/vulkan.h>

namespace vks
{
    class IRenderTarget
    {
    public:
        virtual ~IRenderTarget() = default;

        virtual VkExtent2D extent() const = 0;
        virtual size_t numImages() const = 0;

        virtual VkFormat colorFormat() const = 0;
        virtual VkFormat depthFormat() const = 0;

        virtual VkImageView colorView(uint32_t index) const = 0;
        virtual VkImageView depthView(uint32_t index) const = 0;

        virtual VkImage colorImage(uint32_t index) const = 0;
        virtual VkImage depthImage(uint32_t index) const = 0;
    };
}
