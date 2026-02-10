#pragma once

#include <vulkan/vulkan.h>
#include <render/passes/IRenderPass.hpp>

namespace vks
{
    class ImGuiRenderPass : public IRenderPass
    {
    public:
        ImGuiRenderPass(const Device& device, const SwapChain& swapChain);
        void update(float dt, uint32_t currentImage) override;
        void record(VkCommandBuffer cmd, uint32_t currentImage) override;
        void onResize() override;

        RenderPassType type() const override { return RenderPassType::ImGui; }
    private:
        void createRenderPass() override;
        void createFrameBuffers() override;
    };
} // namespace vks
