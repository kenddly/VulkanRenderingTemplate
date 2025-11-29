#include <array>
#include <iostream>
#include <vks/Device.hpp>
#include <vks/RenderPass.hpp>
#include <vks/SwapChain.hpp>

using namespace vks;

RenderPass::RenderPass(const Device& device, const SwapChain& swapChain)
    : m_renderPass(VK_NULL_HANDLE), m_oldRenderPass(VK_NULL_HANDLE),
      m_device(device), m_swapChain(swapChain)
{
}

RenderPass::~RenderPass()
{
    destroyFrameBuffers();
    vkDestroyRenderPass(m_device.logical(), m_renderPass, nullptr);
}

void RenderPass::recreate()
{
    destroyFrameBuffers();
    m_oldRenderPass = m_renderPass;
    createRenderPass();
    createFrameBuffers();
}

void RenderPass::cleanupOld()
{
    if (m_oldRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(m_device.logical(), m_oldRenderPass, nullptr);
        m_oldRenderPass = VK_NULL_HANDLE;
    }
}

void RenderPass::destroyFrameBuffers()
{
    for (VkFramebuffer& fb : m_frameBuffers)
    {
        vkDestroyFramebuffer(m_device.logical(), fb, nullptr);
    }
}
