#pragma once
#include <render/passes/IRenderPass.hpp>

namespace vks
{
    class UIPass : public IRenderPass
    {
    public:
        UIPass(const Device& device, const SwapChain& swapChain);

        void update(float dt, uint32_t imageIndex) override;
        void record(VkCommandBuffer cmd, uint32_t imageIndex) override;
        void onResize() override;

        void recreate() override;

        RenderPassType type() const override { return RenderPassType::Editor; }

    private:
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;
        std::vector<VkDeviceMemory> m_imageMemory;


        void createImages();
        void createImageViews();
        void createRenderPass() override;
        void createFrameBuffers() override;
    };
}
