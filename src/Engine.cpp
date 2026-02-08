#include <vks/Engine.hpp>
#include <vks/Application.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "vks/EngineContext.hpp"
#include "vks/ImGui/ImGuiRenderPass.hpp"
#include "vks/Render/GeometryPass.hpp"

namespace vks
{
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

    Engine::~Engine()
    {
        m_renderGraph.clear();
        m_assets.clearAll();
    }

    void Engine::registerRenderPass(Ref<IRenderPass> pass)
    {
        m_renderGraph.addPass(pass);
    }

    Ref<DescriptorSetLayout> Engine::getDescriptorSetLayout(const std::string& name) const
    {
        return m_descriptorSetLayouts.at(name);
    }

    void Engine::updateCameraUBO()
    {
        CameraUBO ubo{};
        ubo.view = m_camera.view();
        ubo.proj = m_camera.proj();
        ubo.position = m_camera.getPosition();

        m_cameraUboBuffer->writeToBuffer(&ubo, sizeof(ubo));
    }

    void Engine::drawFrame(bool& framebufferResized)
    {
        if (framebufferResized)
        {
            glm::ivec2 size{};
            m_window.framebufferSize(size);
            m_camera.setAspect(float(size.x) / float(size.y));
        }

        m_renderGraph.execute(framebufferResized);
    }

    void Engine::onInit()
    {
        // "camera" layout (Set 0) for camera UBO
        // Matches: layout(set = 0, binding = 0) uniform CameraUBO
        m_descriptorSetLayouts["camera"] = vks::DescriptorSetLayout::Builder(m_device)
                                           .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                                           .build();

        // "material" layout (Set 1) for material UBO
        // Matches: layout(set = 1, binding = 0) uniform MaterialUBO
        m_descriptorSetLayouts["material"] = vks::DescriptorSetLayout::Builder(m_device)
                                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                         VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT)
                                             .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                         VK_SHADER_STAGE_FRAGMENT_BIT)
                                             .build();
        // Create passes
        auto geometryPass = std::make_shared<GeometryPass>(
            device(),
            renderer().getSwapChain()
        );

        auto imguiPass = std::make_shared<ImGuiRenderPass>(
            device(),
            renderer().getSwapChain()
        );

        registerRenderPass(geometryPass);
        registerRenderPass(imguiPass);

        // Create pipelines

        // Grid pipeline
        GraphicsPipelineDesc gridPipelineDesc{};
        gridPipelineDesc.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        gridPipelineDesc.isVertexInput = false;
        gridPipelineDesc.renderPass = geometryPass->handle();
        gridPipelineDesc.vertexShader = "assets/shaders/grid.vert.spv";
        gridPipelineDesc.fragmentShader = "assets/shaders/grid.frag.spv";
        gridPipelineDesc.alphaBlending = true;
        gridPipelineDesc.cull = VK_CULL_MODE_NONE;
        gridPipelineDesc.depthCompare = VK_COMPARE_OP_LESS;
        gridPipelineDesc.depthTest = true;
        gridPipelineDesc.depthWrite = false;
        gridPipelineDesc.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        gridPipelineDesc.viewportExtent = renderer().getSwapChain().extent();
        gridPipelineDesc.dynamicStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);

        // Sphere pipeline
        GraphicsPipelineDesc spherePipelineDesc{};
        spherePipelineDesc.renderPass = geometryPass->handle();
        spherePipelineDesc.vertexShader = "assets/shaders/sphere.vert.spv";
        spherePipelineDesc.fragmentShader = "assets/shaders/sphere.frag.spv";
        spherePipelineDesc.alphaBlending = false;
        spherePipelineDesc.cull = VK_CULL_MODE_BACK_BIT;
        spherePipelineDesc.depthCompare = VK_COMPARE_OP_LESS;
        spherePipelineDesc.depthTest = true;
        spherePipelineDesc.depthWrite = true;
        spherePipelineDesc.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        spherePipelineDesc.viewportExtent = renderer().getSwapChain().extent();

        // Sprite pipeline
        GraphicsPipelineDesc spritePipelineDesc{};
        spritePipelineDesc.renderPass = geometryPass->handle();
        spritePipelineDesc.vertexShader = "assets/shaders/sprite.vert.spv";
        spritePipelineDesc.fragmentShader = "assets/shaders/sprite.frag.spv";
        spritePipelineDesc.alphaBlending = true;
        spritePipelineDesc.cull = VK_CULL_MODE_NONE;
        spritePipelineDesc.depthCompare = VK_COMPARE_OP_LESS;
        spritePipelineDesc.depthTest = true;
        spritePipelineDesc.depthWrite = true;
        spritePipelineDesc.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        spritePipelineDesc.viewportExtent = renderer().getSwapChain().extent();

        PipelineDesc gridPipelineDesc_{};
        gridPipelineDesc_.type = PipelineType::Graphics;
        gridPipelineDesc_.payload = gridPipelineDesc;
        gridPipelineDesc_.setLayouts = {
            m_descriptorSetLayouts["camera"]->getDescriptorSetLayout(),
            m_descriptorSetLayouts["material"]->getDescriptorSetLayout()
        };

        PipelineDesc spherePipelineDesc_{gridPipelineDesc_};
        spherePipelineDesc_.payload = spherePipelineDesc;
        spherePipelineDesc_.pushConstants = {
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(glm::mat4)
            }
        };

        PipelineDesc spritePipelineDesc_{gridPipelineDesc_};
        spritePipelineDesc_.payload = spritePipelineDesc;
        spritePipelineDesc_.pushConstants = {
            VkPushConstantRange{
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(glm::mat4)
            }
        };

        geometryPass->pipelines().createOrReplace("grid", gridPipelineDesc_);
        geometryPass->pipelines().createOrReplace("sphere", spherePipelineDesc_);
        geometryPass->pipelines().createOrReplace("sprite", spritePipelineDesc_);
    }

    void Engine::run(Application& app)
    {
        // Initialize the engine context for global access
        EngineContext::set(this);
        onInit();

        // Let app setup engine
        app.onInit(*this);

        m_window.setDrawFrameFunc([this, &app](bool& resized, float dt)
        {
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
