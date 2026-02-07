#pragma once

#include <vks/Descriptors.hpp>
#include <vks/Render/IRenderPass.hpp>
#include <FileWatcher.hpp>
#include <ShaderCompiler.hpp>


namespace vks
{
    class GeometryPass : public IRenderPass
    {
    public:
        GeometryPass(const Device& device, const SwapChain& swapChain);

        void update(float dt, uint32_t currentImage) override;
        void record(VkCommandBuffer cmd, uint32_t currentImage) override;
        void onResize() override;

        void recreate() override;

        RenderPassType type() const override { return RenderPassType::Geometry; }
    private:
        void createRenderPass() override;
        void createFrameBuffers() override;

        FileWatcher m_fileWatcher;
        Ref<ShaderCompiler> m_shaderCompiler;
    };
} // namespace vks
