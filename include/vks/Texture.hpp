#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include <vks/Device.hpp>

namespace vks {

    class Texture {
    public:
        Texture(
            const Device& device,
            const std::string& filepath,
            bool generateMipmaps = true
        );

        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        VkImageView getImageView() const { return m_imageView; }
        VkSampler getSampler() const { return m_sampler; }

        VkDescriptorImageInfo descriptorInfo() const;

        uint32_t mipLevels() const { return m_mipLevels; }

        const std::string path;
    private:
        void createTextureImage(const std::string& filepath);
        void createImageView();
        void createSampler();
        void generateMipmaps();

    private:
        const Device& m_device;

        VkImage m_image = VK_NULL_HANDLE;
        VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
        VkImageView m_imageView = VK_NULL_HANDLE;
        VkSampler m_sampler = VK_NULL_HANDLE;

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_mipLevels = 1;
    };

} // namespace vks
