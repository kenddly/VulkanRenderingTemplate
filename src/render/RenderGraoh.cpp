#include <filesystem>
#include <stdexcept>
#include <glm/vec2.hpp>

#include <core/Time.hpp>
#include <app/EngineContext.hpp>
#include <render/RenderGraph.hpp>

namespace vks
{
    bool RenderGraph::execute(bool& framebufferResized)
    {
        // Wait for the previous frame to finish using the 'currentFrame' slot
        vkWaitForFences(device.logical(), 1, &syncObjects.inFlightFence(currentFrame),
                        VK_TRUE, UINT64_MAX);

        // Acquire the next available image from the swapchain
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            device.logical(), swapChain.handle(), UINT64_MAX,
            syncObjects.imageAvailable(currentFrame), VK_NULL_HANDLE, &imageIndex);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain(framebufferResized);
            return framebufferResized;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swapchain image");
        }

        if (syncObjects.imageInFlight(imageIndex) != VK_NULL_HANDLE)
        {
            vkWaitForFences(device.logical(), 1,
                            &syncObjects.imageInFlight(imageIndex), VK_TRUE,
                            UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        syncObjects.imageInFlight(imageIndex) = syncObjects.inFlightFence(currentFrame);

        VkCommandBuffer cmd = commandBuffers.command(currentFrame);

        // Reset the command buffer to clear old commands
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // Run all render passes
        for (auto& pass : m_passes)
        {
            // We pass 'imageIndex' so the RenderPass knows which Framebuffer/Image to draw into.
            // But we write into 'cmd' which belongs to 'currentFrame'.
            pass->record(cmd, imageIndex);
        }

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
        
        submit(cmd, imageIndex);
        present(imageIndex, framebufferResized);
        update(Time::getDeltaTime(), imageIndex);
        
        // 6. Advance Frame
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return framebufferResized;
    }

    void RenderGraph::submit(VkCommandBuffer cmd, uint32_t imageIndex)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // Wait for image to be available
        VkSemaphore waitSemaphores[] = {syncObjects.imageAvailable(currentFrame)};
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        // Submit the command buffer we recorded in execute()
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        // Signal that rendering is finished
        VkSemaphore signalSemaphores[] = {syncObjects.renderFinished(imageIndex)};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(device.logical(), 1, &syncObjects.inFlightFence(currentFrame));
        
        // Signal the fence when GPU completes this frame
        if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo,
                          syncObjects.inFlightFence(currentFrame)) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
    }

    void RenderGraph::present(uint32_t imageIndex, bool& framebufferResized)
    {
        VkSemaphore signalSemaphores[] = {syncObjects.renderFinished(imageIndex)};
            
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain.handle()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
        {
            recreateSwapChain(framebufferResized);
            syncObjects.recreate(swapChain.numImages());
            framebufferResized = false;
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swap chain image");
        }
    }

    void RenderGraph::update(float dt, uint32_t imageIndex)
    {
        for (auto& pass : m_passes)
        {
            pass->update(dt, imageIndex);
        }
    }

    void RenderGraph::recreate()
    {
        for (auto& pass : m_passes)
        {
            pass->recreate();
        }
        // Note: cleanupOld is usually handled inside specific recreate logic or destructors
    }

    void RenderGraph::recreateSwapChain(bool& framebufferResized) const
    {
        framebufferResized = true;

        glm::ivec2 size;
        auto& ce = EngineContext::get();
        auto& window = ce.window();
        window.framebufferSize(size);
        while (size[0] == 0 || size[1] == 0)
        {
            window.framebufferSize(size);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device.logical());

        // 1. Recreate Swapchain
        swapChain.recreate();
        
        // 2. Recreate Passes (Framebuffers, Pipelines)
        for (auto & pass : m_passes)
        {
            pass->recreate();
        }

        // 3. Cleanup old swapchain resources
        swapChain.cleanupOld();
    }
} // namespace vks
