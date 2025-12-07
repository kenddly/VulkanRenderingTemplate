#include <vks/Application.hpp>
#include <vks/Buffer.hpp>
#include <vks/Material.hpp>
#include <vks/Model.hpp>
#include <stdexcept>
#include <iostream>
#include <array>

// Add these for matrix math and time
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <chrono>

#include "vks/GridMaterial.hpp"

using namespace vks;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

vks::Application::Application()
    : instance("Hello Triangle", "No Engine", true),
      debugMessenger(instance),
      window({WIDTH, HEIGHT}, "Vulkan", instance),
      device(instance, window, Instance::DeviceExtensions),
      swapChain(device, window),
      renderPass(device, swapChain),
      commandPool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
      graphicsPipeline(device, swapChain, renderPass),
      commandBuffers(device, renderPass, swapChain, graphicsPipeline, commandPool, *this),
      syncObjects(device, swapChain.numImages(), MAX_FRAMES_IN_FLIGHT),
      interface(instance, window, device, swapChain, graphicsPipeline)
{
    m_app = this;

    // Now that all core systems are up, load assets
    loadAssets();
    buildScene();

    camera.init(&window.input(), swapChain.extent().width / (float)swapChain.extent().height);
}

Application::~Application()
{
    // Release all the assets
    m_assets.clearAll();
}

/**
 * @brief Creates all asset registries (pools, models, materials).
 */
void Application::loadAssets()
{
    // 1. Create Global Descriptor Pool
    m_globalDescriptorPool = vks::DescriptorPool::Builder(device)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100) // For camera + materials
                             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100) // For textures
                             .setMaxSets(200)
                             .build();

    // 2. Create the Camera UBO Buffer
    m_cameraUboBuffer = std::make_unique<vks::Buffer>(
        device,
        sizeof(CameraUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    // Map it persistently. We can write to it at any time.
    m_cameraUboBuffer->map();

    // 3. Create the Camera Descriptor Set (Set 0)
    // (This was the other fix: we create the set that points to the buffer)
    {
        auto globalSetLayout = graphicsPipeline.getDescriptorSetLayout("global");
        auto bufferInfo = m_cameraUboBuffer->descriptorInfo();
        vks::DescriptorWriter(globalSetLayout, m_globalDescriptorPool)
            .writeBuffer(0, &bufferInfo)
            .build(m_cameraDescriptorSet); // m_cameraDescriptorSet is now valid!
    }

    auto sphereModel = Model{};
    sphereModel.createSphere(device, commandPool.handle(), 1.0f, 32, 16);
    m_assets.add<Model>("sphere", std::move(sphereModel));
    
    auto redMaterial = std::make_unique<ColorMaterial>(
        device,
        graphicsPipeline,
        m_globalDescriptorPool,
        "sphere",
        ColorMaterialUBO{ glm::vec4{1.0f, 0.0f, 0.0f, 1.0f} }
    );
    redMaterial->layer_priority = 0;

    auto blueMaterial = std::make_unique<ColorMaterial>(
        device,
        graphicsPipeline,
        m_globalDescriptorPool,
        "sphere",
        ColorMaterialUBO{ glm::vec4{0.0f, 122.0f / 255.0f, 1.0f, 1.0f} } // Blue
    );
    blueMaterial->layer_priority = 0; // Render blue sphere after red sphere

    auto gridMaterial = std::make_unique<GridMaterial>(
        device,
        graphicsPipeline,
        m_globalDescriptorPool,
        "grid",
        GridMaterialUBO{
            glm::vec4{0.0f, 0.67f, 0.78f, 1.0f},
            5.0f,
            20,
            1.0f,
            0.75f,
            1.0f,
            100.0f,
            0.0f
        }
    );

    gridMaterial->layer_priority = -1; // Render grid first
    
    m_assets.add<std::unique_ptr<Material>>("red_sphere", std::move(redMaterial));
    m_assets.add<std::unique_ptr<Material>>("blue_sphere", std::move(blueMaterial));
    m_assets.add<std::unique_ptr<Material>>("grid", std::move(gridMaterial));
}

/**
 * @brief Populates the m_renderObjects list.
 */
void Application::buildScene()
{
    // Create a red sphere at (0, 0, 0)
    RenderObject redSphere{};
    redSphere.model = &m_assets.get<Model>("sphere");
    redSphere.material = m_assets.get<std::unique_ptr<Material>>("red_sphere").get();
    redSphere.transform = glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, 0.0f});
    m_assets.add<RenderObject>("red_sphere", redSphere);

    // Create a blue sphere at (2, 0, 0)
    RenderObject blueSphere{};
    blueSphere.model = &m_assets.get<Model>("sphere");
    blueSphere.material = m_assets.get<std::unique_ptr<Material>>("blue_sphere").get();
    blueSphere.transform = glm::translate(glm::mat4(1.0f), {2.0f, 0.0f, 0.0f});
    m_assets.add<RenderObject>("blue_sphere", blueSphere);

    RenderObject gridObj{};
    gridObj.model = nullptr; // Indicates procedural draw
    gridObj.material = m_assets.get<std::unique_ptr<vks::Material>>("grid").get();
    gridObj.transform = glm::mat4(1.0f);
    m_assets.add<RenderObject>("grid", gridObj);
}

void Application::updateUBOs(uint32_t currentImage)
{
    // --- Update Camera UBO ---
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    CameraUBO ubo{};

    ubo.view = camera.view();
    ubo.proj = camera.proj();
    ubo.position = camera.getPosition();

    // Write to the mapped buffer
    m_cameraUboBuffer->writeToBuffer(&ubo, sizeof(ubo));
}

void Application::run()
{
    window.setDrawFrameFunc([this](bool& framebufferResized, float deltaTime)
    {
        drawImGui();
        drawFrame(framebufferResized);

        camera.update(deltaTime);
        // get all materials and update them
        auto& materials = m_assets.getMap<std::unique_ptr<Material>>();
        for (auto& pair : materials)
            pair.second->update();

        if (window.input().isKeyPressed(GLFW_KEY_SPACE))
        {
            std::cout << "Space key pressed!" << std::endl;
        }
    });

    window.mainLoop();
    vkDeviceWaitIdle(device.logical());
}

std::unordered_map<std::string, RenderObject>& Application::getRenderObjects()
{
    return m_assets.getMap<RenderObject>();
}


void Application::drawFrame(bool& framebufferResized)
{
    vkWaitForFences(device.logical(), 1, &syncObjects.inFlightFence(currentFrame),
                    VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device.logical(), swapChain.handle(), UINT64_MAX,
        syncObjects.imageAvailable(currentFrame), VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain(framebufferResized);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    // Update all UBOs with fresh data for this frame
    // *before* we record the command buffer.
    updateUBOs(currentFrame);

    if (syncObjects.imageInFlight(imageIndex) != VK_NULL_HANDLE)
    {
        vkWaitForFences(device.logical(), 1, &syncObjects.imageInFlight(imageIndex),
                        VK_TRUE, UINT64_MAX);
    }
    syncObjects.imageInFlight(imageIndex) =
        syncObjects.inFlightFence(currentFrame);

    // --- Record the command buffers ---
    // (This will now read the UBO data we just wrote)
    commandBuffers.recordCommands(imageIndex); // Your BasicCommandBuffers
    interface.recordCommandBuffers(imageIndex); // ImGui

    // --- Submit ---
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {syncObjects.imageAvailable(currentFrame)};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // Submit BOTH command buffers (scene and ImGui)
    std::array<VkCommandBuffer, 2> cmdBuffers = {
        commandBuffers.command(imageIndex),
        interface.command(imageIndex)
    };
    submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
    submitInfo.pCommandBuffers = cmdBuffers.data();

    VkSemaphore signalSemaphores[] = {syncObjects.renderFinished(imageIndex)};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device.logical(), 1, &syncObjects.inFlightFence(currentFrame));

    if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo,
                      syncObjects.inFlightFence(currentFrame)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // --- Present ---
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain.handle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        framebufferResized)
    {
        recreateSwapChain(framebufferResized);
        syncObjects.recreate(swapChain.numImages());
        framebufferResized = false;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Application::drawImGui()
{
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // --- NEW MATERIAL EDITOR ---
    ImGui::Begin("Render Objects");

    // We get a reference to the application's map of materials
    auto& materials = m_assets.getMap<RenderObject>();
    for (auto& pair : materials)
    {
        auto& renderObject = pair.second;
        // Create a collapsible "tree node" for each material
        if (ImGui::TreeNode(pair.first.c_str()))
        {
            renderObject.drawImguiEditor();
            ImGui::TreePop();
        }
    }

    ImGui::End(); // End Material Editor

    ImGui::Render();
}

// for resize window
void Application::recreateSwapChain(bool& framebufferResized)
{
    framebufferResized = true;

    glm::ivec2 size;
    window.framebufferSize(size);
    while (size[0] == 0 || size[1] == 0)
    {
        window.framebufferSize(size);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.logical());

    swapChain.recreate();
    renderPass.recreate();
    graphicsPipeline.recreate();
    commandBuffers.recreate();
    interface.recreate();

    renderPass.cleanupOld();
    swapChain.cleanupOld();

    camera.setAspect(swapChain.extent().width / (float)swapChain.extent().height);
}
