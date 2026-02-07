#pragma once

#include <memory>
#include <NonCopyable.hpp>
#include <vector>
#include <vulkan/vulkan.h>

#include "vks/PipelineManager.hpp"

namespace vks
{
    class Device;
    class SwapChain;

    enum class RenderPassType
    {
        Geometry,
        Shadow,
        Lighting,
        PostProcess,
        ImGui,
        Custom
    };

    class IRenderPass : public NonCopyable
    {
    public:
        IRenderPass(const Device& device, const SwapChain& swapChain);
        ~IRenderPass();

        inline const VkRenderPass& handle() const { return m_renderPass; }

        inline const VkFramebuffer& frameBuffer(uint32_t index) const
        {
            return m_frameBuffers[index];
        }

        inline size_t size() const { return m_frameBuffers.size(); }

        virtual RenderPassType type() const = 0;

        virtual void update(float dt, uint32_t currentImage) = 0;
        virtual void record(VkCommandBuffer cmd, uint32_t currentImage) = 0;
        virtual void onResize() = 0;

        virtual void recreate();
        void cleanupOld();

        PipelineManager& pipelines() { return m_pipelineManager; }
        const PipelineManager& pipelines() const { return m_pipelineManager; }

    protected:
        VkRenderPass m_renderPass;
        VkRenderPass m_oldRenderPass;

        std::vector<VkFramebuffer> m_frameBuffers;

        const Device& m_device;
        const SwapChain& m_swapChain;
        PipelineManager m_pipelineManager;

        virtual void createRenderPass() = 0;
        virtual void createFrameBuffers() = 0;

        void destroyFrameBuffers();
    };
} // namespace vks
