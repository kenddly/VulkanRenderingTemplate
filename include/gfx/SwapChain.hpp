#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#include <vulkan/vulkan.h>

#include <core/NonCopyable.hpp>
#include <vector>

#include "render/RenderTarget.hpp"

namespace vks
{
    class Device;
    class Window;

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class SwapChain : public IRenderTarget, NonCopyable
    {
    public:
        explicit SwapChain(const Device& device, const Window& window);
        ~SwapChain() override;

        auto recreate() -> void;
        auto cleanupOld() -> void;

        const VkSwapchainKHR& handle() const { return m_swapChain; }
        VkFormat colorFormat() const override { return m_imageFormat; }
        VkFormat depthFormat() const override { return m_depthFormat; }
        VkExtent2D extent() const override { return m_extent; }
        size_t numImages() const override { return m_images.size(); }
        size_t numImageViews() const { return m_imageViews.size(); }

        const SwapChainSupportDetails& supportDetails() const
        {
            return m_supportDetails;
        }

        VkImageView colorView(uint32_t index) const override
        {
            return m_imageViews[index];
        }

        VkImageView depthView(uint32_t index) const override
        {
            return m_depthViews[index];
        }

        VkImage colorImage(uint32_t index) const override
        {
            return m_images[index];
        }

        inline VkImage depthImage(uint32_t index) const override
        {
            return m_depthImages[index];
        }

        static SwapChainSupportDetails
        QuerySwapChainSupport(const VkPhysicalDevice& device,
                              const VkSurfaceKHR& surface);
        static VkExtent2D
        ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                         const Window& window);

    private:
        const Device& m_device;
        const Window& m_window;

        SwapChainSupportDetails m_supportDetails;
        VkSwapchainKHR m_swapChain;
        VkSwapchainKHR m_oldSwapChain;

        // Swap chain image and depth handles
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;

        std::vector<VkImage> m_depthImages;
        std::vector<VkImageView> m_depthViews;
        std::vector<VkDeviceMemory> m_depthImageMemories;

        VkFormat m_imageFormat;
        VkFormat m_depthFormat;

        VkExtent2D m_extent;

        void createSwapChain();
        void createImageViews();
        void createDepthViews();

        void destroyImages();
        void destroyDepthViews();

        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkFormat ChooseSwapDepthFormat(VkPhysicalDevice device);
        static VkPresentModeKHR ChooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
    };
} // namespace vks

#endif // SWAPCHAIN_HPP
