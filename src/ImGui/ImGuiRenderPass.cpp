
#include <iostream>
#include <vks/Device.hpp>
#include <vks/ImGui/ImGuiRenderPass.hpp>
#include <vks/SwapChain.hpp>

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "vks/Application.hpp"
#include "vks/EngineContext.hpp"

using namespace vks;

ImGuiRenderPass::ImGuiRenderPass(const Device &device,
                                 const SwapChain &swapChain)
    : IRenderPass(device, swapChain) {
    createRenderPass();
    createFrameBuffers();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("assets/fonts/ClearSans-Regular.ttf");
    io.FontGlobalScale = 2.0f;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    auto& ec= EngineContext::get();
    auto& window = ec.window();

    QueueFamilyIndices indices =
        QueueFamily::FindQueueFamilies(m_device.physical(), window.surface());

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window.window(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};

    init_info.Instance = ec.vulkanInstance().handle();
    init_info.PhysicalDevice = m_device.physical();
    init_info.Device = m_device.logical();
    init_info.QueueFamily = indices.graphicsFamily.value();
    init_info.Queue = m_device.graphicsQueue();
    init_info.DescriptorPool = ec.globalDescriptorPool()->getDescriptorPool();
    init_info.MinImageCount = swapChain.numImages();
    init_info.ImageCount = swapChain.numImages();
    init_info.PipelineInfoMain.RenderPass = handle();
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiRenderPass::update(float dt, uint32_t currentImage)
{
}

void ImGuiRenderPass::record(VkCommandBuffer cmd, uint32_t currentImage)
{
    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = handle();
    renderPassBeginInfo.framebuffer = frameBuffer(currentImage);
    renderPassBeginInfo.renderArea.extent.width = m_swapChain.extent().width;
    renderPassBeginInfo.renderArea.extent.height = m_swapChain.extent().height;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmd, &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    // Grab and record the draw data for Dear Imgui
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    // End and submit render pass
    vkCmdEndRenderPass(cmd);
}

void ImGuiRenderPass::onResize()
{
    recreate();
    cleanupOld();
}

void ImGuiRenderPass::createRenderPass() {
  // Create an attachment description for the render pass
  VkAttachmentDescription attachmentDescription = {};
  attachmentDescription.format = m_swapChain.imageFormat();
  attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
  attachmentDescription.loadOp =
      VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Need UI to be drawn on top of main
  attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescription.finalLayout =
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Last pass so we want to present after
  attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Create a color attachment reference
  VkAttachmentReference attachmentReference = {};
  attachmentReference.attachment = 0;
  attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // Create a subpass
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &attachmentReference;

  // Create a subpass dependency to synchronize our main and UI render passes
  // We want to render the UI after the geometry has been written to the
  // framebuffer so we need to configure a subpass dependency as such
  VkSubpassDependency subpassDependency = {};
  subpassDependency.srcSubpass =
      VK_SUBPASS_EXTERNAL;          // Create external dependency
  subpassDependency.dstSubpass = 0; // The geometry subpass comes first
  subpassDependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Wait on writes
  subpassDependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  // Finally create the UI render pass
  VkRenderPassCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  createInfo.attachmentCount = 1;
  createInfo.pAttachments = &attachmentDescription;
  createInfo.subpassCount = 1;
  createInfo.pSubpasses = &subpass;
  createInfo.dependencyCount = 1;
  createInfo.pDependencies = &subpassDependency;

  if (vkCreateRenderPass(m_device.logical(), &createInfo, nullptr,
                         &m_renderPass) != VK_SUCCESS) {
    throw std::runtime_error("Render pass creation failed");
  }
}

void ImGuiRenderPass::createFrameBuffers()
{
    size_t numImages = m_swapChain.numImages();
    m_frameBuffers.resize(numImages);

    for (size_t i = 0; i < numImages; i++)
    {
        VkImageView attachments[] = {
            m_swapChain.imageView(i)
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChain.extent().width;
        framebufferInfo.height = m_swapChain.extent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device.logical(), &framebufferInfo, nullptr,
                                &m_frameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}
