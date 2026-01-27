#pragma once

#include <vks/Render/IRenderPass.hpp>
#include <vks/GeometryPipeline.hpp>
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

        const Ref<GeometryPipeline> getPipeline() const { return m_graphicsPipeline; }

    private:
        void createRenderPass() override;
        void createFrameBuffers() override;

        FileWatcher m_fileWatcher;
        Ref<ShaderCompiler> m_shaderCompiler;
        Ref<GeometryPipeline> m_graphicsPipeline = nullptr;
    };
} // namespace vks
