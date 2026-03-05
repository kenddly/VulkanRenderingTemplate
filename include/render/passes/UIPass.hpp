#pragma once
#include <render/passes/IRenderPass.hpp>

namespace vks
{
    class Buffer;

    class UIPass : public IRenderPass
    {
    public:
        UIPass(const Device& device, const Ref<IRenderTarget>& renderTarget);

        void update(float dt, uint32_t imageIndex) override;
        void record(VkCommandBuffer cmd, uint32_t imageIndex) override;

        void recreate() override;

        RenderPassType type() const override { return RenderPassType::Editor; }

    private:
        std::unique_ptr<Buffer> m_pixelBuffer;

        void createRenderPass() override;
        void createFrameBuffers() override;

        void capturePixelID(VkCommandBuffer cmd, VkImage srcImage, glm::vec2 mousePos);
    };
}
