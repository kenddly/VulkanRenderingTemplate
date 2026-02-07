#pragma once
#include <vector>
#include <memory>
#include <vks/SwapChain.hpp>
#include <vks/SyncObjects.hpp>
#include <vks/CommandBuffers.hpp>
#include <vks/Render/IRenderPass.hpp>
#include <vks/Descriptors.hpp>

namespace vks
{
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    class RenderGraph
    {
    public:
        RenderGraph(const Device& device,
                    SwapChain& swapChain,
                    const CommandPool& commandPool)
            : device(device),
              swapChain(swapChain),
              commandBuffers(device, swapChain, commandPool),
              syncObjects(device, swapChain.numImages(), MAX_FRAMES_IN_FLIGHT)
        {}

        ~RenderGraph()
        {
            clear();
        }

        // Add a pass to the end of the pipeline
        void addPass(Ref<IRenderPass> pass)
        {
            m_passes.push_back(pass);
        }

        template<typename T> Ref<T> getPass(RenderPassType type) const
        {
            for (const auto& pass : m_passes)
            {
                if (pass->type() == type)
                    return std::dynamic_pointer_cast<T>(pass);
            }
            return nullptr;
        }

        // Execute all passes in order
        bool execute(bool& framebufferResized);

        void recreate();
        void recreateSwapChain(bool& framebufferResized) const;

        // Cleanup
        void clear() { m_passes.clear(); }

        SwapChain& getSwapChain() { return swapChain; }

    private:
        void submit(VkCommandBuffer cmd, uint32_t imageIndex);
        void present(uint32_t imageIndex, bool& framebufferResized);
        void update(float dt, uint32_t imageIndex);
        
        uint32_t currentFrame = 0;

        const Device& device;
        SwapChain& swapChain;

        std::vector<Ref<IRenderPass>> m_passes;
        CommandBuffers commandBuffers;

        SyncObjects syncObjects;
    };

}
