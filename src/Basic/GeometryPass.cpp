#include <vks/Render/GeometryPass.hpp>
#include <vks/Device.hpp>
#include <vks/SwapChain.hpp>
#include <vks/Application.hpp>

#include <array>
#include <iostream>

using namespace vks;

GeometryPass::GeometryPass(const Device &device,
                                 const SwapChain &swapChain)
    : IRenderPass(device, swapChain)
{
    GeometryPass::createRenderPass();
    GeometryPass::createFrameBuffers();
    m_graphicsPipeline = std::make_shared<GeometryPipeline>(device, swapChain, handle());

    m_shaderCompiler = std::make_shared<ShaderCompiler>([&](const std::filesystem::path& path)
    {
        std::cout << "Recreating graphics pipeline..." << std::endl;
        vkDeviceWaitIdle(m_device.logical());
        m_graphicsPipeline->recreate();
    });
    
    FileWatcher::Callback callback = [&](const std::filesystem::path &path)
    {
        std::cout << "Shader changed: " << path << ", recompiling..." << std::endl;
        m_shaderCompiler->requestCompile(path);
    };
    
    m_fileWatcher.watchDirectory("assets/shaders/", {".frag", ".vert"}, false, callback);
    
    m_graphicsPipeline->recreate();
}

void GeometryPass::update(float dt, uint32_t currentImage)
{
    m_fileWatcher.update();
    m_shaderCompiler->update();
}

void GeometryPass::record(VkCommandBuffer cmdBuffer, uint32_t imageIndex)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = handle();
    renderPassInfo.framebuffer = frameBuffer(imageIndex);
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
    auto& app = Application::getInstance();
    auto renderObjects = app.getRenderObjects();
    VkDescriptorSet cameraSet = app.getCameraDescriptorSet();
    const auto& camera = app.getCamera(); // Need this for Grid

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
        auto layout = m_graphicsPipeline->getLayout(layoutName);
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                layout, 0, 1, &cameraSet, 0, nullptr);
    }

    // Render Loop
    VkPipeline lastPipeline = VK_NULL_HANDLE;
    VkDescriptorSet lastMaterialSet = VK_NULL_HANDLE;

    for (const auto& obj : renderObjects)
    {
        auto& renderObject = obj.second;
        auto pipelineName = renderObject.material->getPipelineName();
        VkPipeline pipeline = m_graphicsPipeline->getPipeline(pipelineName);
        VkPipelineLayout layout = m_graphicsPipeline->getLayout(pipelineName);

        // --- Bind Pipeline (If Changed) ---
        if (pipeline != lastPipeline)
        {
            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            lastPipeline = pipeline;

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
}

void GeometryPass::onResize()
{
}

void GeometryPass::recreate()
{
    m_graphicsPipeline->recreate();
    IRenderPass::recreate();
}

void GeometryPass::createRenderPass()
{
    // Color attachment
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_swapChain.imageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth Attachment
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = m_swapChain.depthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef = {};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    // We now wait for both Color Output AND Early Fragment Tests (Depth)
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;

    // We want to write Color AND write Depth
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Create Render Pass
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = attachments.size();
    createInfo.pAttachments = attachments.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device.logical(), &createInfo, nullptr,
                           &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Render pass creation failed");
    }
}

void GeometryPass::createFrameBuffers()
{
    size_t numImages = m_swapChain.numImages();

    m_frameBuffers.resize(numImages);

    // Create a framebuffer for each image view
    for (size_t i = 0; i < numImages; ++i)
    {
        std::array<VkImageView, 2> attachments = {m_swapChain.imageView(i), m_swapChain.depthView(i)};

        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = m_renderPass;
        info.attachmentCount = attachments.size();
        info.pAttachments = attachments.data();
        info.width = m_swapChain.extent().width;
        info.height = m_swapChain.extent().height;
        info.layers = 1;

        if (vkCreateFramebuffer(m_device.logical(), &info, nullptr,
                                &m_frameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Framebuffer creation failed");
        }
    }
}
