#include <gfx/CommandBuffers.hpp>
#include <gfx/CommandPool.hpp>
#include <gfx/Device.hpp>
#include <gfx/SwapChain.hpp>
#include <app/EngineContext.hpp>

#include <stdexcept>

using namespace vks;

CommandBuffers::CommandBuffers(const Device &device,
                               const SwapChain &swapChain,
                               const CommandPool &commandPool)
    : m_device(device), m_swapChain(swapChain),
      m_commandPool(commandPool)
{
    createCommandBuffers(swapChain.numImages());
}

CommandBuffers::~CommandBuffers() { destroyCommandBuffers(); }

void CommandBuffers::createCommandBuffers(uint32_t count)
{
    m_commandBuffers.resize(count);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool.handle();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

    if (vkAllocateCommandBuffers(m_device.logical(), &allocInfo,
                                 m_commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CommandBuffers::destroyCommandBuffers() {
  vkFreeCommandBuffers(m_device.logical(), m_commandPool.handle(),
                       static_cast<uint32_t>(m_commandBuffers.size()),
                       m_commandBuffers.data());
}

void CommandBuffers::SingleTimeCommands(
    const Device &device,
    const std::function<void(const VkCommandBuffer &)> &func) {
  auto& cmdPool = EngineContext::get().commandPool();
  VkCommandBufferAllocateInfo allocInfo = {};

  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = cmdPool.handle();
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer = {};
  if (vkAllocateCommandBuffers(device.logical(), &allocInfo, &commandBuffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("Could not create one-time command buffer!");
  }

  func(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(device.graphicsQueue());

  vkFreeCommandBuffers(device.logical(), cmdPool.handle(), 1, &commandBuffer);
}

void CommandBuffers::startRecording(uint32_t index)
{
    auto cmdBuffer = m_commandBuffers[index];
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void CommandBuffers::stopRecording(uint32_t index)
{
    VkCommandBuffer cmd = m_commandBuffers[index];

    // End Recording
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandBuffers::recreate() {
    destroyCommandBuffers();
    createCommandBuffers(m_swapChain.numImages());
}

