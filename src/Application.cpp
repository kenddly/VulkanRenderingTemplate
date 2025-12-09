#include <vks/Application.hpp>
#include <vks/Buffer.hpp>
#include <vks/Material.hpp>
#include <vks/Model.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <chrono>

#include "Time.hpp"
#include "vks/GridMaterial.hpp"
#include <vks/Render/GeometryPass.hpp>
#include <vks/ImGui/ImGuiRenderPass.hpp>


using namespace vks;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

vks::Application::Application()
    : instance("Hello Triangle", "No Engine", true),
      debugMessenger(instance),
      window({WIDTH, HEIGHT}, "Vulkan", instance),
      device(instance, window, Instance::DeviceExtensions),
      swapChain(device, window),
      commandPool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
      renderGraph(device, swapChain, commandPool)
{
    m_app = this;

    camera.init(&window.input(), swapChain.extent().width / (float)swapChain.extent().height);

    auto geometryPass = std::make_unique<GeometryPass>(device, swapChain);
    graphicsPipeline = geometryPass->getPipeline(); // Expose pipeline manager
    renderGraph.addPass(std::move(geometryPass));

    // Now that all core systems are up, load assets
    loadAssets();
    buildScene();

    // add the ImGui pass
    auto imguiPass = std::make_unique<ImGuiRenderPass>(device, swapChain);
    renderGraph.addPass(std::move(imguiPass));
}

Application::~Application()
{
    renderGraph.clear();
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
                             .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
                             .setMaxSets(1000 * 11) // 11 types
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
        auto globalSetLayout = graphicsPipeline->getDescriptorSetLayout("global");
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
        *graphicsPipeline.get(),
        m_globalDescriptorPool,
        "sphere",
        ColorMaterialUBO{glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}}
    );
    redMaterial->layer_priority = 0;

    auto blueMaterial = std::make_unique<ColorMaterial>(
        device,
        *graphicsPipeline.get(),
        m_globalDescriptorPool,
        "sphere",
        ColorMaterialUBO{glm::vec4{0.0f, 122.0f / 255.0f, 1.0f, 1.0f}} // Blue
    );
    blueMaterial->layer_priority = 0; // Render blue sphere after red sphere

    auto gridMaterial = std::make_unique<GridMaterial>(
        device,
        *graphicsPipeline.get(),
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

void Application::updateUBOs()
{
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
        updateUBOs();

        // get all materials and update them
        auto& materials = m_assets.getMap<std::unique_ptr<Material>>();
        for (auto& pair : materials)
            pair.second->update();

        // Physics::calculateGravity();
        Time::setDeltaTime(deltaTime);
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
    bool frameBufferResized = renderGraph.execute(framebufferResized);
    if (frameBufferResized)
    {
        camera.setAspect(swapChain.extent().width / (float)swapChain.extent().height);
    }
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
