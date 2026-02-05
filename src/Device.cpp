#include <vks/Device.hpp>

#include <vks/Instance.hpp>
#include <vks/Window.hpp>

#include <map>
#include <set>
#include <vector>

#include <vks/QueueFamily.hpp>
#include <vks/SwapChain.hpp>

#include "vks/CommandBuffers.hpp"

using namespace vks;

Device::Device(const Instance& instance, const Window& window,
               const std::vector<const char*>& extensions)
    : m_physical(VK_NULL_HANDLE), m_logical(VK_NULL_HANDLE), m_window(window),
      m_instance(instance), m_graphicsQueue(VK_NULL_HANDLE),
      m_presentQueue(VK_NULL_HANDLE)
{
    m_physical =
        PickPhysicalDevice(m_instance.handle(), m_window.surface(), extensions);
    m_indices = QueueFamily::FindQueueFamilies(m_physical, m_window.surface());

    // Setup queue families for device
    std::set<uint32_t> uniqueQueueFamilies = {
        m_indices.graphicsFamily.value(),
        m_indices.presentFamily.value()
    };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float priority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.queueFamilyIndex = queueFamily;
        createInfo.queueCount = 1;
        createInfo.pQueuePriorities = &priority;
        queueCreateInfos.push_back(createInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.wideLines = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // Setup logical device
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (m_instance.validationLayersEnabled())
    {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(Instance::ValidationLayers.size());
        createInfo.ppEnabledLayerNames = Instance::ValidationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physical, &createInfo, nullptr, &m_logical) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    // Get handles for graphics and presentation queues
    vkGetDeviceQueue(m_logical, m_indices.graphicsFamily.value(), 0,
                     &m_graphicsQueue);
    vkGetDeviceQueue(m_logical, m_indices.presentFamily.value(), 0,
                     &m_presentQueue);

    vkGetPhysicalDeviceProperties(m_physical, &m_properties);
}

Device::~Device() { vkDestroyDevice(m_logical, nullptr); }

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    // Query available memory types on the physical device
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physical, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        // 1. Check if the memory type is allowed by the buffer/image requirements (typeFilter)
        // 2. Check if the memory type supports the properties we need (e.g., Host Visible, Device Local)
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

bool Device::CheckDeviceExtensionSupport(
    const VkPhysicalDevice& device,
    const std::vector<const char*>& extensions)
{
    // Get number of extension supported
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);

    if (extensionCount < 1)
        return false;

    // Get supported extensions
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    // Iterate through available extensions and make sure that all are present
    std::set<std::string> requiredExtensions(extensions.begin(),
                                             extensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VkPhysicalDevice Device::PickPhysicalDevice(
    const VkInstance& instance, const VkSurfaceKHR& surface,
    const std::vector<const char*>& requiredExtensions)
{
    // Check for devices with vulkan support
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    // Get available devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    for (const auto& device : devices)
    {
        if (IsDeviceSuitable(device, surface))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    return physicalDevice;
}

bool Device::IsDeviceSuitable(const VkPhysicalDevice& device,
                              const VkSurfaceKHR& surface)
{
    QueueFamilyIndices indices = QueueFamily::FindQueueFamilies(device, surface);

    bool extensionsSupported =
        CheckDeviceExtensionSupport(device, Instance::DeviceExtensions);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport =
            SwapChain::QuerySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() &&
            !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void vks::Device::createImage(
    uint32_t width,
    uint32_t height,
    uint32_t mipLevels,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory
) const
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(logical(), &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logical(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        memRequirements.memoryTypeBits,
        properties
    );

    if (vkAllocateMemory(logical(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate image memory");

    vkBindImageMemory(logical(), image, imageMemory, 0);
}

VkImageView vks::Device::createImageView(
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags,
    uint32_t mipLevels
) const
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;

    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(logical(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view");

    return imageView;
}

void vks::Device::transitionImageLayout(
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    uint32_t mipLevels
) const
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::runtime_error("Unsupported image layout transition");
    }

    CommandBuffers::SingleTimeCommands(*this, [&](const VkCommandBuffer& cmd)
    {
        vkCmdPipelineBarrier(
            cmd,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    });
}

void vks::Device::copyBufferToImage(
    VkBuffer buffer,
    VkImage image,
    uint32_t width,
    uint32_t height
) const
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    CommandBuffers::SingleTimeCommands(*this, [&](const VkCommandBuffer& cmd)
    {
        vkCmdCopyBufferToImage(
            cmd,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    });
}

void Device::generateMipmaps(VkImage image, VkFormat imageFormat, uint32_t width, uint32_t height,
                             uint32_t mipLevels) const
{
    // Check format support
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(
        physical(), imageFormat, &props
    );

    if (!(props.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("Texture format does not support linear blitting");
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    CommandBuffers::SingleTimeCommands(*this, [&](const VkCommandBuffer& cmd)
    {
        for (uint32_t i = 1; i < mipLevels; i++)
        {
            // Transition previous level to SRC
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            // Blit
            VkImageBlit blit{};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;

            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {
                mipWidth > 1 ? mipWidth / 2 : 1,
                mipHeight > 1 ? mipHeight / 2 : 1,
                1
            };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(
                cmd,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR
            );

            // Transition previous level to SHADER_READ
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            mipWidth = std::max(mipWidth / 2, 1);
            mipHeight = std::max(mipHeight / 2, 1);
        }

        // Transition last mip level
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    });
}
