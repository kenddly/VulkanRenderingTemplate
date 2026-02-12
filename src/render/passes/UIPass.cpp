#include <glm/fwd.hpp>
#include <render/passes/UIPass.hpp>

#include "app/EngineContext.hpp"
#include "editor/UI/components/Renderable.hpp"
#include "gfx/Device.hpp"
#include "gfx/SwapChain.hpp"

namespace vks
{
    UIPass::UIPass(const Device& device, const SwapChain& swapChain)
        : IRenderPass(device, swapChain)
    {
        createImages();
        createImageViews();
        UIPass::createRenderPass();
        UIPass::createFrameBuffers();
    }

    void UIPass::update(float dt, uint32_t imageIndex)
    {
    }

    void UIPass::record(VkCommandBuffer cmd, uint32_t imageIndex)
    {
        VkRenderPassBeginInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        info.renderPass = handle();
        info.framebuffer = frameBuffer(imageIndex);
        info.renderArea.offset = {0, 0};
        info.renderArea.extent = m_swapChain.extent();

        std::array<VkClearValue, 2> clears{};
        clears[0].color = {{0, 0, 0, 0}};
        clears[1].depthStencil = {1.0f, 0};

        info.clearValueCount = clears.size();
        info.pClearValues = clears.data();

        vkCmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.width = (float)m_swapChain.extent().width;
        viewport.height = (float)m_swapChain.extent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{{0, 0}, m_swapChain.extent()};
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        auto& ce = EngineContext::get();
        auto view = ce.scene().view<Renderable, Transform>();

        VkPipeline pipeline = pipelines().getPipeline("ObjectPicker");
        VkPipelineLayout layout = pipelines().getLayout("ObjectPicker");

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // Bind camera descriptor set
        VkDescriptorSet cameraSet = ce.cameraDescriptorSet();
        if (cameraSet != VK_NULL_HANDLE && view.size_hint() != 0)
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &cameraSet, 0, nullptr);

        for (auto entity : view)
        {
            auto& renderable = view.get<Renderable>(entity);
            auto& transform = view.get<Transform>(entity);

            if (!renderable.model) continue;

            struct PushData
            {
                glm::mat4 model;
                uint32_t id;
            } pushData;
            pushData.model = transform.transform;
            pushData.id = (uint32_t)entity + 1;

            vkCmdPushConstants(
                cmd,
                layout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PushData),
                &pushData
            );

            renderable.model->bind(cmd);
        }

        vkCmdEndRenderPass(cmd);
    }

    void UIPass::onResize()
    {
    }

    void UIPass::recreate()
    {
        createImages();
        createImageViews();
        IRenderPass::recreate();
    }

    // Create iamges with R32 format for object ID storage
    void UIPass::createImages()
    {
        m_images.resize(m_swapChain.numImages());
        m_imageMemory.resize(m_swapChain.numImages());

        for (size_t i = 0; i < m_images.size(); i++)
        {
            VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
            info.imageType = VK_IMAGE_TYPE_2D;
            info.format = VK_FORMAT_R32_UINT;
            info.extent.width = m_swapChain.extent().width;
            info.extent.height = m_swapChain.extent().height;
            info.extent.depth = 1;
            info.mipLevels = 1;
            info.arrayLayers = 1;
            info.samples = VK_SAMPLE_COUNT_1_BIT;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.usage =
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            vkCreateImage(m_device.logical(), &info, nullptr, &m_images[i]);

            // 🔹 Query memory requirements
            VkMemoryRequirements memReq;
            vkGetImageMemoryRequirements(m_device.logical(), m_images[i], &memReq);

            VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            allocInfo.allocationSize = memReq.size;
            allocInfo.memoryTypeIndex =
                m_device.findMemoryType(
                    memReq.memoryTypeBits,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                );

            vkAllocateMemory(
                m_device.logical(),
                &allocInfo,
                nullptr,
                &m_imageMemory[i]
            );

            // Bind memory
            vkBindImageMemory(
                m_device.logical(),
                m_images[i],
                m_imageMemory[i],
                0
            );
        }
    }

    void UIPass::createImageViews()
    {
        m_imageViews.resize(m_images.size());

        for (size_t i = 0; i < m_images.size(); i++)
        {
            VkImageViewCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            info.image = m_images[i];
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = VK_FORMAT_R32_UINT;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            vkCreateImageView(m_device.logical(), &info, nullptr, &m_imageViews[i]);
        }
    }

    void UIPass::createRenderPass()
    {
        VkAttachmentDescription color{};
        color.format = VK_FORMAT_R32_UINT;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; // IMPORTANT

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &color;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;

        vkCreateRenderPass(
            m_device.logical(),
            &info,
            nullptr,
            &m_renderPass
        );
    }

    void UIPass::createFrameBuffers()
    {
        m_frameBuffers.resize(m_swapChain.numImages());

        for (uint32_t i = 0; i < m_frameBuffers.size(); i++)
        {
            VkImageView attachments[] = {m_imageViews[i]};

            VkFramebufferCreateInfo fb{};
            fb.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fb.renderPass = m_renderPass;
            fb.attachmentCount = 1;
            fb.pAttachments = attachments;
            fb.width = m_swapChain.extent().width;
            fb.height = m_swapChain.extent().height;
            fb.layers = 1;

            vkCreateFramebuffer(
                m_device.logical(),
                &fb,
                nullptr,
                &m_frameBuffers[i]
            );
        }
    }
}
