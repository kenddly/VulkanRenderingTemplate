#pragma once
#include <vector>
#include <memory>
#include <gfx/SwapChain.hpp>
#include <gfx/SyncObjects.hpp>
#include <gfx/CommandBuffers.hpp>
#include <render/passes/IRenderPass.hpp>


namespace vks
{
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    class RenderGraph
    {
    public:
        RenderGraph(const Device& device,
                    const Ref<SwapChain>& swapChain,
                    const CommandPool& commandPool);

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
        void execute();

        void recreatePasses();
        void recreate();

        // Cleanup
        void clear() { m_passes.clear(); }

        Ref<SwapChain> getSwapChain() const { return swapChain; }

        uint32_t getCurrentFrameIndex() const { return currentFrame; }
        uint32_t getCurrentImageIndex() const { return currentFrame; }

    private:
        void submit(VkCommandBuffer cmd);
        void present(uint32_t imageIndex);
        void update(float dt, uint32_t imageIndex);
        
        uint32_t currentFrame = 0;
        uint32_t imageIndex = 0;

        const Device& device;
        const Ref<SwapChain> swapChain;

        std::vector<Ref<IRenderPass>> m_passes;
        CommandBuffers commandBuffers;

        SyncObjects syncObjects;
    };

}
