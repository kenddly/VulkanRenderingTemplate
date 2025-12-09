#include <array>
#include <iostream>
#include <vks/Device.hpp>
#include <../include/vks/Render/IRenderPass.hpp>
#include <vks/SwapChain.hpp>

using namespace vks;

IRenderPass::IRenderPass(const Device& device, const SwapChain& swapChain)
    : m_renderPass(VK_NULL_HANDLE), m_oldRenderPass(VK_NULL_HANDLE),
      m_device(device), m_swapChain(swapChain)
{
}

IRenderPass::~IRenderPass()
{
    destroyFrameBuffers();
    vkDestroyRenderPass(m_device.logical(), m_renderPass, nullptr);
}

void IRenderPass::recreate()
{
    destroyFrameBuffers();
    m_oldRenderPass = m_renderPass;
    createRenderPass();
    createFrameBuffers();
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
