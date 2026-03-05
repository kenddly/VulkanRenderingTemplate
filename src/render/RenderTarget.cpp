#include <render/RenderTarget.hpp>
#include <gfx/Device.hpp>
#include <spdlog/spdlog.h>

#include "imgui_impl_vulkan.h"
#include "core/Log.hpp"

namespace vks
{
    RenderTarget::RenderTarget(
        const Device& device,
        VkExtent2D extent,
        uint32_t imageCount,
        VkFormat colorFormat,
        VkFormat depthFormat,
        VkImageUsageFlags additionalUsage,
        bool sampled
    )
        : m_device(device),
          m_extent(extent),
          m_imageCount(imageCount),
          m_colorFormat(colorFormat),
          m_depthFormat(depthFormat),
          m_sampled(sampled),
          m_additionalUsage(additionalUsage)
    {
        if (extent.width == 0 || extent.height == 0)
            return;

        createImages();
        createImageViews();

        if (sampled)
        {
            createSampler();
        }
    }

    RenderTarget::~RenderTarget()
    {
        destroy();
    }

    void RenderTarget::resize(VkExtent2D newExtent)
    {
        if (newExtent.width == 0 || newExtent.height == 0)
        {
            LOG_WARN("Detected viewport extent of size 0,0");
            return;
        }

        if (newExtent.width == m_extent.width &&
            newExtent.height == m_extent.height)
            return;

        vkDeviceWaitIdle(m_device.logical());
        destroy();

        m_extent = newExtent;

        createImages();
        createImageViews();
        if (m_sampled) createDescriptors();
    }

    VkDescriptorSet RenderTarget::renderTargetImage(uint32_t index)
    {
        if (m_viewportDescriptors.empty())
            createDescriptors();
        return m_viewportDescriptors[index];
    }

    void RenderTarget::createDescriptors()
    {
        m_viewportDescriptors.clear();
        m_viewportDescriptors.resize(m_imageCount);

        for (int i = 0; i < m_imageCount; ++i)
        {
            m_viewportDescriptors[i] = ImGui_ImplVulkan_AddTexture(m_sampler,
                                                                   colorView(i),
                                                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }

    void RenderTarget::createImages()
    {
        m_colorImages.resize(m_imageCount);
        m_colorMemory.resize(m_imageCount);

        m_depthImages.resize(m_imageCount);
        m_depthMemory.resize(m_imageCount);

        VkImageUsageFlags colorUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | m_additionalUsage |
            (m_sampled ? VK_IMAGE_USAGE_SAMPLED_BIT : 0);

        for (uint32_t i = 0; i < m_imageCount; i++)
        {
            m_device.createImage(
                m_extent.width,
                m_extent.height,
                1,
                m_colorFormat,
                VK_IMAGE_TILING_OPTIMAL,
                colorUsage,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_colorImages[i],
                m_colorMemory[i]
            );

            m_device.createImage(
                m_extent.width,
                m_extent.height,
                1,
                m_depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_depthImages[i],
                m_depthMemory[i]
            );
        }
    }

    void RenderTarget::createImageViews()
    {
        m_colorViews.resize(m_imageCount);
        m_depthViews.resize(m_imageCount);

        for (uint32_t i = 0; i < m_imageCount; i++)
        {
            m_colorViews[i] = m_device.createImageView(
                m_colorImages[i],
                m_colorFormat,
                VK_IMAGE_ASPECT_COLOR_BIT,
                1
            );

            m_depthViews[i] = m_device.createImageView(
                m_depthImages[i],
                m_depthFormat,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                1
            );
        }
    }

    void RenderTarget::createSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        vkCreateSampler(m_device.logical(), &samplerInfo, nullptr, &m_sampler);
    }

    void RenderTarget::destroy()
    {
        VkDevice device = m_device.logical();

        for (uint32_t i = 0; i < m_colorViews.size(); i++)
        {
            vkDestroyImageView(device, m_colorViews[i], nullptr);
            vkDestroyImage(device, m_colorImages[i], nullptr);
            vkFreeMemory(device, m_colorMemory[i], nullptr);

            vkDestroyImageView(device, m_depthViews[i], nullptr);
            vkDestroyImage(device, m_depthImages[i], nullptr);
            vkFreeMemory(device, m_depthMemory[i], nullptr);
        }

        for (auto& desc : m_viewportDescriptors)
        {
            if (desc != VK_NULL_HANDLE)
                ImGui_ImplVulkan_RemoveTexture(desc);
        }

        m_colorViews.clear();
        m_colorImages.clear();
        m_colorMemory.clear();
        m_depthViews.clear();
        m_depthImages.clear();
        m_depthMemory.clear();
    }
}
