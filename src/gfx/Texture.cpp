#include <../include/gfx/Texture.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <cmath>

#include <../include/gfx/Buffer.hpp>

namespace vks
{
    Texture::Texture(
        const Device& device,
        const std::string& filepath,
        bool generateMipmaps
    )
        : m_device(device),
            path(filepath)
    {
        createTextureImage(filepath);
        createImageView();
        createSampler();
    }

    Texture::~Texture()
    {
        VkDevice device = m_device.logical();

        if (m_sampler) vkDestroySampler(device, m_sampler, nullptr);
        if (m_imageView) vkDestroyImageView(device, m_imageView, nullptr);
        if (m_image) vkDestroyImage(device, m_image, nullptr);
        if (m_imageMemory) vkFreeMemory(device, m_imageMemory, nullptr);
    }

    void Texture::createTextureImage(const std::string& filepath)
    {
        int texWidth, texHeight, texChannels;

        stbi_uc* pixels = stbi_load(
            filepath.c_str(),
            &texWidth,
            &texHeight,
            &texChannels,
            STBI_rgb_alpha
        );

        if (!pixels)
        {
            throw std::runtime_error("Failed to load texture image: " + filepath);
        }

        m_width = texWidth;
        m_height = texHeight;
        m_mipLevels = static_cast<uint32_t>(
            std::floor(std::log2(std::max(m_width, m_height))) + 1
        );

        VkDeviceSize imageSize = m_width * m_height * 4;

        Buffer stagingBuffer(
            m_device,
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(pixels);
        stagingBuffer.unmap();

        stbi_image_free(pixels);

        m_device.createImage(
            m_width,
            m_height,
            m_mipLevels,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_image,
            m_imageMemory
        );

        m_device.transitionImageLayout(
            m_image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_mipLevels
        );

        m_device.copyBufferToImage(
            stagingBuffer.getBuffer(),
            m_image,
            m_width,
            m_height
        );

        m_device.generateMipmaps(
            m_image,
            VK_FORMAT_R8G8B8A8_SRGB,
            m_width,
            m_height,
            m_mipLevels
        );
    }

    void Texture::createImageView()
    {
        m_imageView = m_device.createImageView(
            m_image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT,
            m_mipLevels
        );
    }

    void Texture::createSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = m_device.properties().limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(m_mipLevels);
        samplerInfo.mipLodBias = 0.0f;

        if (vkCreateSampler(m_device.logical(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture sampler");
        }
    }

    VkDescriptorImageInfo Texture::descriptorInfo() const
    {
        VkDescriptorImageInfo info{};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageView = m_imageView;
        info.sampler = m_sampler;
        return info;
    }
} // namespace vks
