#include <vks/Basic/BasicCommandBuffers.hpp>
#include <vks/Application.hpp>
#include <stdexcept>
#include <array>
#include <algorithm>

using namespace vks;

BasicCommandBuffers::BasicCommandBuffers(
    const Device& device, const RenderPass& renderPass,
    const SwapChain& swapChain, const GraphicsPipeline& graphicsPipeline,
    const CommandPool& commandPool,
    Application& application // <-- ADD THIS
)
    : CommandBuffers(device, renderPass, swapChain, graphicsPipeline, commandPool),
      m_app(application) // <-- STORE THIS
{
    BasicCommandBuffers::createCommandBuffers();
}

void BasicCommandBuffers::recreate()
{
    destroyCommandBuffers();
    createCommandBuffers();
}

void BasicCommandBuffers::createCommandBuffers()
{
    m_commandBuffers.resize(m_renderPass.size());

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

struct PushData
{
    glm::vec4 camPosition;
    glm::vec4 color;
    glm::vec3 settings;
};


/**
 * @brief This is the new "cooking" function that renders your scene.
 * It is called every frame from Application::drawFrame.
 */
void BasicCommandBuffers::recordCommands(uint32_t imageIndex)
{
    VkCommandBuffer cmdBuffer = m_commandBuffers[imageIndex];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass.handle();
    renderPassInfo.framebuffer = m_renderPass.frameBuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain.extent();

    // Set clear color AND depth
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0}; // For depth buffer
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChain.extent().width);
    viewport.height = static_cast<float>(m_swapChain.extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChain.extent();
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    // Get Scene Data
    auto renderObjects = m_app.getRenderObjects();
    VkDescriptorSet cameraSet = m_app.getCameraDescriptorSet();
    const auto& camera = m_app.getCamera(); // Need this for Grid

    // Sort (Optimization)
    // TODO: Reimplement this somehow
    // std::sort(renderObjects.begin(), renderObjects.end(),
    //           [](const RenderObject& a, const RenderObject& b)
    //           {
    //               return a.getSortKey() < b.getSortKey();
    //           });

    // Bind Global Camera Set (Set 0)
    if (cameraSet != VK_NULL_HANDLE && !renderObjects.empty())
    {
        auto renderObject = renderObjects.begin();
        auto layoutName = renderObject->second.material->getPipelineName();
        auto layout = m_graphicsPipeline.getLayout(layoutName);
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout, 0, 1, &cameraSet, 0, nullptr);
    }

    // Render Loop
    VkPipeline lastPipeline = VK_NULL_HANDLE;
    VkPipelineLayout lastLayout = VK_NULL_HANDLE;
    VkDescriptorSet lastMaterialSet = VK_NULL_HANDLE;

    for (const auto& obj : renderObjects)
    {
        auto& renderObject = obj.second;
        auto pipelineName = renderObject.material->getPipelineName();
        VkPipeline pipeline = m_graphicsPipeline.getPipeline(pipelineName);
        VkPipelineLayout layout = m_graphicsPipeline.getLayout(pipelineName);

        // --- Bind Pipeline (If Changed) ---
        if (pipeline != lastPipeline)
        {
            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            lastPipeline = pipeline;
            lastLayout = layout;

            // Re-bind global set if layout changed (Vulkan requirement)
            if (cameraSet != VK_NULL_HANDLE)
            {
                vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        layout, 0, 1, &cameraSet, 0, nullptr);
            }
        }

        renderObject.material->draw(
            cmdBuffer,
            layout,
            lastMaterialSet, // Passed by reference so material can update cache
            renderObject.model,
            renderObject.transform,
            camera
        );
    }

    vkCmdEndRenderPass(cmdBuffer);

    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}
