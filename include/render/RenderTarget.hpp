#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

#include <render/IRenderTarget.hpp>

namespace vks
{
    class Device;

    class RenderTarget : public IRenderTarget
    {
    public:
        RenderTarget(
            const Device& device,
            VkExtent2D extent,
            uint32_t imageCount,
            VkFormat colorFormat,
            VkFormat depthFormat,
            bool sampled = true
        );

        ~RenderTarget() override;

        void resize(VkExtent2D newExtent);

        VkExtent2D extent() const override { return m_extent; }

        size_t numImages() const override { return m_imageCount; }

        // Per-frame access
        VkImageView colorView(uint32_t index) const override { return m_colorViews[index]; }
        VkImageView depthView(uint32_t index) const override { return m_depthViews[index]; }
        VkSampler imageSampler() const { return m_sampler; }

        VkImage colorImage(uint32_t index) const override { return m_colorImages[index]; }

        VkFormat colorFormat() const override { return m_colorFormat; }
        VkFormat depthFormat() const override { return m_depthFormat; }
        VkDescriptorSet renderTargetImage(uint32_t index);

        // Viewport descriptor sets
        std::vector<VkDescriptorSet> m_viewportDescriptors{};
        void createViewportDescriptors();
    protected:
        void createImages();
        void createImageViews();
        void createSampler();
        void destroy();

        const Device& m_device;

        VkExtent2D m_extent{};
        size_t m_imageCount{};

        VkFormat m_colorFormat{};
        VkFormat m_depthFormat{};
        bool m_sampled{true};

        std::vector<VkImage> m_colorImages;
        std::vector<VkDeviceMemory> m_colorMemory;
        std::vector<VkImageView> m_colorViews;

        std::vector<VkImage> m_depthImages;
        std::vector<VkDeviceMemory> m_depthMemory;
        std::vector<VkImageView> m_depthViews;

        VkSampler m_sampler{};
    };
}
