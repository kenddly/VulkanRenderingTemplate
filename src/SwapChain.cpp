#include <vks/SwapChain.hpp>

#include <vks/Device.hpp>
#include <vks/Window.hpp>

#include <iostream>

using namespace vks;

SwapChain::SwapChain(const Device& device, const Window& window)
  : m_swapChain(VK_NULL_HANDLE), m_oldSwapChain(VK_NULL_HANDLE), m_extent(),
    m_imageFormat(), m_device(device), m_window(window)
{
  createSwapChain();
  createImageViews();
  createDepthViews();
}

void SwapChain::recreate()
{
  destroyImages();
  m_oldSwapChain = m_swapChain;
  createSwapChain();
  createImageViews();
  createDepthViews();
}

void SwapChain::cleanupOld()
{
  // Destroy old swap chain if it exists
  if (m_oldSwapChain != VK_NULL_HANDLE)
  {
    vkDestroySwapchainKHR(m_device.logical(), m_oldSwapChain, nullptr);
    m_oldSwapChain = VK_NULL_HANDLE;
  }
}

void SwapChain::createSwapChain()
{
  m_supportDetails =
    QuerySwapChainSupport(m_device.physical(), m_window.surface());
  m_extent = ChooseSwapExtent(m_supportDetails.capabilities, m_window);

  VkSurfaceFormatKHR surfaceFormat =
    ChooseSwapSurfaceFormat(m_supportDetails.formats);
  VkFormat depthFormat = ChooseSwapDepthFormat(m_device.physical());
  VkPresentModeKHR presentMode =
    ChooseSwapPresentMode(m_supportDetails.presentModes);

  m_imageFormat = surfaceFormat.format;
  m_depthFormat = depthFormat;

  // How many images should be in the swap chain
  // One more than the minimum helps with wait times before another image is
  // available from driver
  uint32_t imageCount = m_supportDetails.capabilities.minImageCount + 1;

  // Make sure image count doesn't exceed maximum
  // A max image count of 0 indicates that there is no maximum
  if (m_supportDetails.capabilities.maxImageCount > 0 &&
    imageCount > m_supportDetails.capabilities.maxImageCount)
  {
    imageCount = m_supportDetails.capabilities.maxImageCount;
  }

  // Setup the swapchain
  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_window.surface();

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = m_extent;
  // Amount of layers in each image
  // This is always 1 unless doing something like stereoscopic 3D
  createInfo.imageArrayLayers = 1;
  // Image is being directly rendered to, so make i a color attachment
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // How to handle swap chain images across multiple queue families
  const QueueFamilyIndices& indices = m_device.queueFamilyIndices();
  uint32_t queueFamilyIndices[] = {
    indices.graphicsFamily.value(),
    indices.presentFamily.value()
  };

  // Determine if there are multiple queue families
  if (indices.graphicsFamily != indices.presentFamily)
  {
    // Use concurrent mode if there are. Worse performance but no ownership
    // transfers
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    // Use exclusive mode if single queue family. Best performance
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }

  // Don't do any extra transformations to images
  createInfo.preTransform = m_supportDetails.capabilities.currentTransform;
  // Blend with other windows in window system
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  // Clip obscured pixels
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = m_oldSwapChain;

  if (vkCreateSwapchainKHR(m_device.logical(), &createInfo, nullptr,
                           &m_swapChain) != VK_SUCCESS)
  {
    throw std::runtime_error("Swap chain creation failed");
  }

  // Get new swap chain images
  vkGetSwapchainImagesKHR(m_device.logical(), m_swapChain, &imageCount,
                          nullptr);
  m_images.resize(imageCount);
  vkGetSwapchainImagesKHR(m_device.logical(), m_swapChain, &imageCount,
                          m_images.data());
}

VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(
  const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
  // Try to find support for 8-bit SRGB instead
  for (const auto& availableFormat : availableFormats)
  {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
      availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      return availableFormat;
    }
  }

  // Default to first available format
  return availableFormats[0];
}

VkFormat SwapChain::ChooseSwapDepthFormat(VkPhysicalDevice device)
{
  std::vector formats = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT
  };

  for (VkFormat format : formats)
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(device, format, &props);

    // We require optimal tiling for depth attachments
    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) ==
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
      return format;
    }
  }

  throw std::runtime_error("Failed to find a supported depth format!");
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode(
  const std::vector<VkPresentModeKHR>& availablePresentModes)
{
  // Try to find support for Mailbox
  // MAILBOX (triple buffering) uses a queue to present images,
  // and if the queue is full already queued images are overwritten with newer
  // images
  for (const auto& availablePresentMode : availablePresentModes)
  {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
    {
      return availablePresentMode;
    }
  }

  // Otherwise default to VK_PRESENT_MODE_FIFO_KHR
  // FIFO is guaranteed to be present and is essentially traditional V-Sync
  return VK_PRESENT_MODE_FIFO_KHR;
}

void SwapChain::createImageViews()
{
  m_imageViews.resize(m_images.size());

  for (size_t i = 0; i < m_images.size(); i++)
  {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_images[i];

    // How image should be interpreted
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = m_imageFormat;

    // Leave swizzling as default
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // Purpose of image and which parts should be accessed
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device.logical(), &createInfo, nullptr,
                          &m_imageViews[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

void SwapChain::createDepthViews()
{
  VkFormat availableFormat = ChooseSwapDepthFormat(m_device.physical());
  m_depthFormat = availableFormat;

  VkExtent2D swapChainExtent = extent();

  // Resize vectors to match the number of color images (usually 2 or 3)
  m_depthImages.resize(numImages());
  m_depthViews.resize(numImages());
  m_depthImageMemories.resize(numImages());

  for (int i = 0; i < m_depthImages.size(); i++)
  {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = availableFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    if (vkCreateImage(m_device.logical(), &imageInfo, nullptr, &m_depthImages[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create depth image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device.logical(), m_depthImages[i], &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_device.findMemoryType(memRequirements.memoryTypeBits,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device.logical(), &allocInfo, nullptr, &m_depthImageMemories[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate depth image memory!");
    }

    vkBindImageMemory(m_device.logical(), m_depthImages[i], m_depthImageMemories[i], 0);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_depthImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = availableFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device.logical(), &viewInfo, nullptr, &m_depthViews[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create depth image view!");
    }
  }
}


void SwapChain::destroyImages()
{
  for (VkImageView& view : m_imageViews)
  {
    vkDestroyImageView(m_device.logical(), view, nullptr);
  }
  for (size_t i = 0; i < m_depthImages.size(); i++)
  {
    vkDestroyImageView(m_device.logical(), m_depthViews[i], nullptr);
    vkDestroyImage(m_device.logical(), m_depthImages[i], nullptr);
    vkFreeMemory(m_device.logical(), m_depthImageMemories[i], nullptr);
  }
}

void SwapChain::destroyDepthViews()
{
  for (VkImageView& view : m_depthViews)
  {
    vkDestroyImageView(m_device.logical(), view, nullptr);
  }
}


SwapChain::~SwapChain()
{
  destroyImages();
  vkDestroySwapchainKHR(m_device.logical(), m_swapChain, nullptr);
}

SwapChainSupportDetails
SwapChain::QuerySwapChainSupport(const VkPhysicalDevice& device,
                                 const VkSurfaceKHR& surface)
{
  SwapChainSupportDetails details;

  // Get capabilities of both device and surface
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  // Get number of supported surface formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  // Get supported surface formats
  if (formatCount > 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.formats.data());
  }

  // Get number of supported presentation modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                            nullptr);

  if (presentModeCount > 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkExtent2D
SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                            const Window& window)
{
  // Vulkan uses uint32 max value to signify window resolution should be used
  if (capabilities.currentExtent.width != UINT32_MAX)
  {
    return capabilities.currentExtent;
  }
  // Otherwise, the window manager allows a custom resolution
  else
  {
    VkExtent2D actualExtent = {
      static_cast<uint32_t>(window.dimensions().x),
      static_cast<uint32_t>(window.dimensions().y)
    };

    // Determine if resolution given by vulkan or custom window resolution is
    // the better fit
    actualExtent.width = std::max(
      capabilities.minImageExtent.width,
      std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
      capabilities.minImageExtent.height,
      std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}
