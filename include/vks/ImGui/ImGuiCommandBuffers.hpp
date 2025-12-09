#ifndef IMGUICOMMANDBUFFERS_HPP
#define IMGUICOMMANDBUFFERS_HPP

#include <vector>
#include <vks/CommandBuffers.hpp>
#include <vulkan/vulkan.h>

namespace vks
{
    class ImGuiCommandBuffers : public CommandBuffers
    {
    public:
        ImGuiCommandBuffers(const Device& device,
                            const SwapChain& swapChain,
                            const CommandPool& commandPool);

        void recordCommandBuffers(VkCommandBuffer cmd, uint32_t imageIndex);
        void recreate();

    private:
        void createCommandBuffers();
    };
} // namespace vks

#endif // IMGUICOMMANDBUFFERS_HPP
