#pragma once

#include <stdexcept>
#include <vks/render/IRenderPass.hpp>
#include <vulkan/vulkan.h>

namespace vks
{
    class ImGuiRenderPass : public IRenderPass
    {
    public:
        ImGuiRenderPass(const Device& device, const SwapChain& swapChain);
        void update(float dt, uint32_t currentImage) override;
        void record(VkCommandBuffer cmd, uint32_t currentImage) override;
        void onResize() override;
        
    private:
        void createRenderPass() override;
        void createFrameBuffers() override;
    };
} // namespace vks
