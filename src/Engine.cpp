#include <vks/Engine.hpp>
#include <vks/Application.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "vks/EngineContext.hpp"

namespace vks {

Engine::Engine(const EngineConfig& config)
    : m_instance(config.appName, config.engineName, config.enableValidation),
      m_debugMessenger(m_instance),
      m_window({config.width, config.height}, config.appName, m_instance),
      m_device(m_instance, m_window, Instance::DeviceExtensions),
      m_swapChain(m_device, m_window),
      m_commandPool(m_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
      m_renderGraph(m_device, m_swapChain, m_commandPool)
{
    // Camera
    m_camera.init(
        &m_window.input(),
        m_swapChain.extent().width / float(m_swapChain.extent().height)
    );

    // Global Descriptor Pool
    m_globalDescriptorPool = DescriptorPool::Builder(m_device)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
        .setMaxSets(1000)
        .build();

    // Camera UBO
    m_cameraUboBuffer = std::make_shared<Buffer>(
        m_device,
        sizeof(CameraUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    m_cameraUboBuffer->map();

}

Engine::~Engine() {
    m_renderGraph.clear();
    m_assets.clearAll();
}

void Engine::registerRenderPass(Ref<IRenderPass> pass) {
    m_renderGraph.addPass(pass);
}

void Engine::updateCameraUBO() {
    CameraUBO ubo{};
    ubo.view = m_camera.view();
    ubo.proj = m_camera.proj();
    ubo.position = m_camera.getPosition();

    m_cameraUboBuffer->writeToBuffer(&ubo, sizeof(ubo));
}

void Engine::drawFrame(bool& framebufferResized) {
    if (framebufferResized) {
        glm::ivec2 size{};
        m_window.framebufferSize(size);
        m_camera.setAspect(float(size.x) / float(size.y));
    }

    m_renderGraph.execute(framebufferResized);
}

void Engine::run(Application& app) {
    // Initialize the engine context for global access
    EngineContext::set(this);

    // Let app setup engine
    app.onInit(*this);

    m_window.setDrawFrameFunc([this, &app](bool& resized, float dt) {
        // App logic
        app.tick();

        // Engine logic
        m_camera.update(dt);
        updateCameraUBO();

        // UI
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        app.onImGui();

        ImGui::Render();

        // Render
        drawFrame(resized);
    });

    m_window.mainLoop();
    vkDeviceWaitIdle(m_device.logical());
}

}
