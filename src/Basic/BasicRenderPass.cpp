#include <iostream>
#include <vks/Basic/BasicRenderPass.hpp>
#include <vks/Device.hpp>
#include <vks/SwapChain.hpp>

#include <array>

using namespace vks;

BasicRenderPass::BasicRenderPass(const Device &device,
                                 const SwapChain &swapChain)
    : RenderPass(device, swapChain) {
  createRenderPass();
  createFrameBuffers();
}


void BasicRenderPass::createRenderPass()
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

    // -----------------------------------------------------------------
    // 5. Create Render Pass
    // -----------------------------------------------------------------
    // Combine color and depth into an array
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 2;
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


void BasicRenderPass::createFrameBuffers()
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


