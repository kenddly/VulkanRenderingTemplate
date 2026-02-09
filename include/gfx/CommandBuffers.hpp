#ifndef COMMANDBUFFERS_HPP
#define COMMANDBUFFERS_HPP

#include <core/NonCopyable.hpp>
#include <functional>
#include <vector>
#include <gfx/CommandPool.hpp>
#include <gfx/SwapChain.hpp>
#include <vulkan/vulkan.h>


namespace vks
{
    class CommandBuffers : public NonCopyable
    {
    public:
        CommandBuffers(const Device& device,
                       const SwapChain& swapChain,
                       const CommandPool& commandPool);
        ~CommandBuffers();

        inline VkCommandBuffer& command(uint32_t index)
        {
            return m_commandBuffers[index];
        }

        inline const VkCommandBuffer& command(uint32_t index) const
        {
            return m_commandBuffers[index];
        }

        void recreate();
        static void SingleTimeCommands(const Device& device,
                           const std::function<void(const VkCommandBuffer&)>& func);

        void startRecording(uint32_t index);
        void stopRecording(uint32_t index);
        
    protected:
        std::vector<VkCommandBuffer> m_commandBuffers;

        const Device& m_device;
        const SwapChain& m_swapChain;
        const CommandPool& m_commandPool;

        void createCommandBuffers(uint32_t count);
        void destroyCommandBuffers();
    };
} // namespace vks

#endif // COMMANDBUFFERS_HPP
