#pragma once

#include <NonCopyable.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace vks
{
    class Device;
    class SwapChain;

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

        virtual void update(float dt, uint32_t currentImage) = 0;
        virtual void record(VkCommandBuffer cmd, uint32_t currentImage) = 0;
        virtual void onResize() = 0;

        virtual void recreate();
        void cleanupOld();

    protected:
        VkRenderPass m_renderPass;
        VkRenderPass m_oldRenderPass;

        std::vector<VkFramebuffer> m_frameBuffers;

        const Device& m_device;
        const SwapChain& m_swapChain;

        virtual void createRenderPass() = 0;
        virtual void createFrameBuffers() = 0;

        void destroyFrameBuffers();
    };
} // namespace vks
