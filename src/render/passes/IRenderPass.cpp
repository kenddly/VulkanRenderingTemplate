#include <gfx/Device.hpp>
#include <gfx/SwapChain.hpp>
#include <render/passes/IRenderPass.hpp>

using namespace vks;

IRenderPass::IRenderPass(const Device& device, const Ref<IRenderTarget>& renderTarget)
    : m_renderPass(VK_NULL_HANDLE), m_oldRenderPass(VK_NULL_HANDLE),
      m_device(device), m_renderTarget(renderTarget), m_pipelineManager(device)
{
}

IRenderPass::~IRenderPass()
{
    destroyFrameBuffers();
    vkDestroyRenderPass(m_device.logical(), m_renderPass, nullptr);
}

void IRenderPass::recreate()
{
    vkDeviceWaitIdle(m_device.logical());
    destroyFrameBuffers();
    m_oldRenderPass = m_renderPass;
    createRenderPass();
    createFrameBuffers();
    pipelines().recreateAll();
}

void IRenderPass::cleanupOld()
{
    if (m_oldRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(m_device.logical(), m_oldRenderPass, nullptr);
        m_oldRenderPass = VK_NULL_HANDLE;
    }
}

void IRenderPass::destroyFrameBuffers()
{
    for (VkFramebuffer& fb : m_frameBuffers)
    {
        vkDestroyFramebuffer(m_device.logical(), fb, nullptr);
    }
}
